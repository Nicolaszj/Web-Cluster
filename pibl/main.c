/*
 * main.c - Punto de entrada del PIBL (Proxy Inverso + Balanceador)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "logger.h"
#include "config.h"
#include "round_robin.h"
#include "http_proxy.h"
#include "cache.h"

/* Estructura para pasar datos a los hilos */
typedef struct {
    int  sockfd;
    char ip_cliente[INET_ADDRSTRLEN];
    ConfigProxy *config;
} DatosHilo;

/* Instancia global de configuracion */
static ConfigProxy g_config;

/* Manejador de señal para cierre limpio */
void manejar_sigint(int sig) {
    (void)sig;
    log_info("Cerrando Proxy...");
    logger_close();
    exit(0);
}

void *atender_cliente(void *arg) {
    DatosHilo *datos = (DatosHilo *)arg;
    
    /* Lógica principal del proxy: forwarding */
    gestionar_proxy(datos->sockfd, datos->ip_cliente, datos->config);
    
    close(datos->sockfd);
    free(datos);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ARCHIVO_CONFIG> <ARCHIVO_LOG>\n", argv[0]);
        return 1;
    }

    /* 1. Inicializar Logger */
    if (logger_init(argv[2]) < 0) return 1;

    /* 2. Cargar Configuracion */
    if (cargar_configuracion(argv[1], &g_config) < 0) {
        logger_close();
        return 1;
    }

    /* 2.5 Inicializar Cache */
    cache_init();

    /* 3. Configurar señal de interrupción */
    signal(SIGINT, manejar_sigint);

    log_info("=== PIBL - Proxy Inverso + Balanceador ===");
    log_info("Puerto Escucha: %d", g_config.puerto_escucha);
    log_info("Backends: %d", g_config.num_backends);

    /* 4. Crear socket servidor */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear socket");
        logger_close();
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(g_config.puerto_escucha);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error en bind");
        close(server_fd);
        logger_close();
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("Error en listen");
        close(server_fd);
        logger_close();
        return 1;
    }

    log_info("Proxy listo y escuchando...");

    /* 5. Bucle de aceptacion */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd < 0) {
            log_error("Error en accept");
            continue;
        }

        DatosHilo *datos = malloc(sizeof(DatosHilo));
        if (datos == NULL) {
            log_error("Sin memoria para nuevo hilo");
            close(client_fd);
            continue;
        }

        datos->sockfd = client_fd;
        datos->config = &g_config;
        inet_ntop(AF_INET, &client_addr.sin_addr, datos->ip_cliente, INET_ADDRSTRLEN);

        pthread_t hilo;
        if (pthread_create(&hilo, NULL, atender_cliente, datos) != 0) {
            log_error("Error al crear hilo");
            free(datos);
            close(client_fd);
            continue;
        }
        pthread_detach(hilo);
    }

    close(server_fd);
    logger_close();
    return 0;
}
