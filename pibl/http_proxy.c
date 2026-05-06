/*
 * http_proxy.c - Implementacion de la logica de forwarding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "http_proxy.h"
#include "http_parser.h"
#include "round_robin.h"
#include "logger.h"
#include "cache.h"

#define BUFFER_SIZE 8192
#define MAX_CACHE_SIZE (1024 * 1024) /* 1MB */

void gestionar_proxy(int client_fd, const char *client_ip, ConfigProxy *config) {
    char buffer[BUFFER_SIZE];
    ssize_t leidos = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (leidos <= 0) return;
    buffer[leidos] = '\0';

    /* 2. Parsear la peticion para saber el metodo y la URI */
    PeticionHttp req;
    memset(&req, 0, sizeof(PeticionHttp));
    
    /* Usamos el parser de TWS (ajustado para trabajar con el buffer ya leido) */
    /* Como el parser de TWS lee del socket, necesitamos una version que lea de buffer 
       o simplemente parsear manualmente la primera linea aqui. */
    char linea[1024];
    char metodo[16], uri[1024], version[16];
    sscanf(buffer, "%15s %1023s %15s", metodo, uri, version);

    /* 3. Verificar Cache (Solo para GET) */
    int es_get = (strcmp(metodo, "GET") == 0);
    if (es_get) {
        char *cache_buf = malloc(MAX_CACHE_SIZE);
        size_t cache_size = MAX_CACHE_SIZE;
        if (cache_buscar(uri, config->cache_ttl, cache_buf, &cache_size) == 0) {
            log_info("CACHE HIT: Sirviendo %s desde disco", uri);
            send(client_fd, cache_buf, cache_size, 0);
            free(cache_buf);
            return;
        }
        free(cache_buf);
        log_info("CACHE MISS: %s", uri);
    }

    /* 4. Elegir backend usando Round Robin */
    Backend *backend = obtener_siguiente_backend(config);
    if (backend == NULL) {
        log_error("No hay backends disponibles");
        return;
    }

    /* 5. Conectar al backend */
    int backend_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_fd < 0) return;

    struct sockaddr_in b_addr;
    memset(&b_addr, 0, sizeof(b_addr));
    b_addr.sin_family = AF_INET;
    b_addr.sin_port = htons(backend->port);
    inet_pton(AF_INET, backend->host, &b_addr.sin_addr);

    if (connect(backend_fd, (struct sockaddr *)&b_addr, sizeof(b_addr)) < 0) {
        log_error("Fallo conexion a backend %s:%d", backend->host, backend->port);
        close(backend_fd);
        const char *error_502 = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
        send(client_fd, error_502, strlen(error_502), 0);
        return;
    }

    /* 6. Enviar peticion al backend */
    send(backend_fd, buffer, leidos, 0);

    /* 7. Recibir respuesta y Cachear si es necesario */
    char *full_response = NULL;
    size_t total_resp_size = 0;
    if (es_get) {
        full_response = malloc(MAX_CACHE_SIZE);
    }

    char relay_buf[BUFFER_SIZE];
    ssize_t n;
    while ((n = recv(backend_fd, relay_buf, sizeof(relay_buf), 0)) > 0) {
        send(client_fd, relay_buf, n, 0);
        
        /* Acumular para el cache si hay espacio */
        if (es_get && full_response && (total_resp_size + n < MAX_CACHE_SIZE)) {
            memcpy(full_response + total_resp_size, relay_buf, n);
            total_resp_size += n;
        }
    }

    if (es_get && full_response && total_resp_size > 0) {
        cache_guardar(uri, full_response, total_resp_size);
    }

    if (full_response) free(full_response);
    close(backend_fd);
}
