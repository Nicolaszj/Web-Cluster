/*
 * main.c — Motor principal del Proxy Inverso y Balanceador de Carga (PIBL)
 * Proyecto: PIBL-WS  |  Rol 2: @Nicolaszj
 *
 * Este archivo integra los módulos de configuración y caché para cumplir
 * con los requerimientos del PDF:
 *  - Escucha en puerto parametrizado (8080).
 *  - Balanceo Round Robin entre múltiples backends.
 *  - Caché persistente en disco con TTL.
 *  - Concurrencia mediante hilos (Thread-per-connection).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"
#include "cache.h"

/* ---- Estado Global ---------------------------------------------------- */

Config g_cfg;
int    g_next_backend = 0;
pthread_mutex_t g_mutex_backend = PTHREAD_MUTEX_INITIALIZER;

/* ---- Estructuras de Hilo ---------------------------------------------- */

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} ClientArgs;

/* ---- Utilidades de Red ------------------------------------------------ */

/**
 * Conecta a un servidor backend.
 */
int conectar_a_backend(const Backend *be) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(be->puerto);
    
    if (inet_pton(AF_INET, be->ip, &server_addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

/* ---- Lógica de Hilo --------------------------------------------------- */

void *atender_cliente(void *arg) {
    ClientArgs *args = (ClientArgs *)arg;
    int client_fd = args->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &args->client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    free(args);

    char buffer[8192];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    /* 
     * Simplificación del parseo para el Proxy:
     * Extraer Método, Host, Path y Query para la caché.
     */
    char metodo[16], path[256], proto[16], host[64], query[128] = "";
    sscanf(buffer, "%s %s %s", metodo, path, proto);
    
    /* Buscar el header Host */
    char *host_ptr = strstr(buffer, "Host: ");
    if (host_ptr) {
        sscanf(host_ptr + 6, "%s", host);
    } else {
        strcpy(host, "unknown");
    }

    /* Separar Path y Query */
    char *q_ptr = strchr(path, '?');
    if (q_ptr) {
        *q_ptr = '\0';
        strcpy(query, q_ptr + 1);
    }

    printf("[PROXY] %s solicita %s\n", client_ip, path);

    /* --- 1. Consultar Caché --- */
    CacheResultado cache_res = cache_lookup(metodo, host, path, query);
    if (cache_res == CACHE_HIT) {
        size_t size;
        char *cached_data = cache_load(metodo, host, path, query, &size);
        if (cached_data) {
            send(client_fd, cached_data, size, 0);
            free(cached_data);
            printf("[PROXY] Servido desde CACHE\n");
            close(client_fd);
            return NULL;
        }
    }

    /* --- 2. Round Robin: Seleccionar Backend --- */
    pthread_mutex_lock(&g_mutex_backend);
    Backend be = g_cfg.backends[g_next_backend];
    g_next_backend = (g_next_backend + 1) % g_cfg.num_backends;
    pthread_mutex_unlock(&g_mutex_backend);

    printf("[PROXY] Forwarding a backend %s:%d\n", be.ip, be.puerto);

    /* --- 3. Forwarding al Backend --- */
    int backend_fd = conectar_a_backend(&be);
    if (backend_fd < 0) {
        char *err_502 = "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, err_502, strlen(err_502), 0);
        close(client_fd);
        return NULL;
    }

    /* Enviar petición original al backend */
    send(backend_fd, buffer, bytes_read, 0);

    /* Recibir respuesta del backend (y guardarla para la caché si aplica) */
    char *resp_full = malloc(2 * 1024 * 1024); // Buffer de 2MB para acumular (puedes ajustarlo)
    size_t total_resp_size = 0;
    char chunk[8192];
    ssize_t bytes_in;

    while ((bytes_in = recv(backend_fd, chunk, sizeof(chunk), 0)) > 0) {
        /* Enviar al cliente de inmediato para mayor fluidez */
        send(client_fd, chunk, bytes_in, 0);
        
        /* Acumular para la caché si hay espacio */
        if (total_resp_size + bytes_in < 2 * 1024 * 1024) {
            memcpy(resp_full + total_resp_size, chunk, bytes_in);
            total_resp_size += bytes_in;
        }
    }
    
    if (total_resp_size > 0) {
        /* Intentar guardar en caché si es GET 200 OK */
        if (cache_es_cacheable(metodo, 200)) { 
            cache_store(metodo, host, path, query, resp_full, total_resp_size);
        }
    }
    free(resp_full);

    close(backend_fd);
    close(client_fd);
    return NULL;
}

/* ---- Main Loop -------------------------------------------------------- */

int main(int argc, char *argv[]) {
    const char *config_file = (argc > 1) ? argv[1] : "config.txt";

    if (config_cargar(config_file, &g_cfg) != 0) {
        fprintf(stderr, "Error al cargar configuracion: %s\n", config_file);
        return 1;
    }

    if (cache_init("./cache", g_cfg.ttl) != 0) {
        return 1;
    }

    config_imprimir(&g_cfg);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(g_cfg.puerto);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error en bind");
        return 1;
    }

    listen(server_fd, 128);
    printf("[PROXY] PIBL escuchando en el puerto %d...\n", g_cfg.puerto);

    while (1) {
        ClientArgs *args = malloc(sizeof(ClientArgs));
        socklen_t addr_len = sizeof(args->client_addr);
        args->client_fd = accept(server_fd, (struct sockaddr *)&args->client_addr, &addr_len);
        
        if (args->client_fd >= 0) {
            pthread_t thread;
            pthread_create(&thread, NULL, atender_cliente, args);
            pthread_detach(thread);
        } else {
            free(args);
        }
    }

    cache_destroy();
    config_destruir(&g_cfg);
    return 0;
}
