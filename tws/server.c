/*
 * server.c - Punto de entrada del TWS (Telematics Web Server)
 *
 * Este archivo es el "pegamento" del servidor: inicializa todo, abre el
 * socket, y por cada conexion entrante lanza un hilo que la atiende.
 *
 * Concurrencia: Thread-per-connection
 * -------------------------------------------------
 * Por cada cliente que se conecta, creamos un hilo POSIX (pthread).
 * El hilo vive el tiempo que dure la conexion y luego muere.
 *
 * Alternativas evaluadas:
 *   1. Thread pool (grupo fijo de hilos esperando trabajo):
 *      Mas eficiente en servidores de alto trafico porque evita crear/destruir
 *      hilos continuamente. Requiere una cola de trabajo y un mutex/semaforo.
 *      Descartado por complejidad; el enunciado no lo exige.
 *
 *   2. select() / poll() / epoll() (I/O no bloqueante, un solo hilo):
 *      El mecanismo de Nginx y Node.js. Muy eficiente pero radicalmente mas
 *      complejo de implementar y entender. El enunciado pide "Thread Based".
 *
 *   3. fork() (un proceso hijo por conexion):
 *      Cada proceso tiene su propio espacio de memoria → no necesita mutex
 *      para el logger. Pero un proceso es mucho mas pesado que un hilo
 *      (copia de toda la memoria del padre). Descartado.
 *
 *   4. pthread_detach (lo que usamos):
 *      Creamos el hilo y lo "desvinculamos". Esto significa que cuando el
 *      hilo termina, el SO libera sus recursos automaticamente sin que
 *      nadie tenga que hacer pthread_join(). Perfecto para el caso de
 *      conexiones independientes donde no necesitamos el valor de retorno.
 *
 * Ejecucion:
 *   ./server <PUERTO> <ARCHIVO_LOG> <CARPETA_RAIZ>
 *   ./server 8080 /var/log/tws.log /var/www/html
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
#include "http_parser.h"
#include "http_response.h"


/*
 * Estructura que se pasa a cada hilo al crearlo.
 * Por que usamos malloc() para esto? Porque el hilo es asincronico:
 * si pusieramos esta estructura en el stack del main loop y el loop
 * avanzara a la siguiente iteracion antes de que el hilo la leyera,
 * los datos se pisarian. Con malloc(), la memoria es del hilo y el
 * hilo la libera cuando ya no la necesita.
 */
typedef struct {
    int  sockfd;
    char ip_cliente[INET_ADDRSTRLEN];
} DatosConexion;


/*
 * Funcion que ejecuta cada hilo.
 * La firma void* f(void*) es la que exige pthread_create().
 * El argumento es un puntero a DatosConexion que casteamos de vuelta.
 */
static void *atender_conexion(void *arg)
{
    DatosConexion *datos = (DatosConexion *)arg;
    int sockfd = datos->sockfd;
    char ip[INET_ADDRSTRLEN];
    strcpy(ip, datos->ip_cliente);
    free(datos);  /* liberar la memoria que nos paso el loop principal */

    /* Parsear la peticion HTTP del socket */
    PeticionHttp peticion;
    int resultado = parsear_peticion(sockfd, &peticion);

    if (resultado == 0) {
        /*
         * Se recibieron datos del socket. Si la peticion es invalida,
         * procesar_peticion() se encarga de responder 400.
         */
        procesar_peticion(sockfd, &peticion, ip);
    }
    /* Si resultado == -1: el cliente se desconecto antes de enviar nada.
     * No hay nada que responder. */

    liberar_peticion(&peticion);

    /*
     * Cerrar el socket del cliente.
     * El servidor usa Connection: close en todas las respuestas, entonces
     * tras enviar la respuesta el socket ya no sirve para nada.
     * El servidor principal (server_fd) NO se cierra aqui, ese es del main.
     */
    close(sockfd);
    return NULL;
}


int main(int argc, char *argv[])
{
    /* --- Validacion de argumentos --- */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <PUERTO> <ARCHIVO_LOG> <CARPETA_RAIZ>\n",
                argv[0]);
        fprintf(stderr, "Ejemplo: %s 8080 servidor.log /var/www/html\n",
                argv[0]);
        return 1;
    }

    int puerto = atoi(argv[1]);
    if (puerto < 1 || puerto > 65535) {
        fprintf(stderr, "Error: puerto invalido '%s'. Rango valido: 1-65535.\n",
                argv[1]);
        return 1;
    }

    /* --- Inicializar el logger --- */
    if (logger_init(argv[2]) < 0) {
        /* El error ya fue impreso por logger_init con perror() */
        return 1;
    }

    /* --- Guardar la ruta raiz de documentos (variable global en http_response.c) --- */
    strncpy(g_doc_root, argv[3], sizeof(g_doc_root) - 1);
    g_doc_root[sizeof(g_doc_root) - 1] = '\0';

    log_info("=== Telematics Web Server (TWS) ===");
    log_info("Puerto     : %d", puerto);
    log_info("Log        : %s", argv[2]);
    log_info("DocumentRoot: %s", g_doc_root);

    /*
     * --- Crear el socket del servidor ---
     *
     * socket(AF_INET, SOCK_STREAM, 0):
     *   AF_INET    → familia de direcciones IPv4 (vs AF_INET6 para IPv6)
     *   SOCK_STREAM → TCP (orientado a conexion, confiable, ordenado)
     *                 La alternativa seria SOCK_DGRAM para UDP, que no tiene
     *                 conexion ni garantias de entrega → no sirve para HTTP
     *   0          → protocolo por defecto para la familia+tipo elegidos (TCP)
     */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear el socket");
        logger_close();
        return 1;
    }

    /*
     * SO_REUSEADDR: permite que el servidor se reinicie sin esperar que el
     * sistema operativo libere el puerto (que puede tardar hasta 2 minutos
     * en estado TIME_WAIT de TCP). Sin esta opcion, reiniciar el servidor
     * da "Address already in use" aunque no haya nadie usando el puerto.
     */
    int opcion = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));

    /*
     * --- Configurar la direccion del servidor ---
     * sockaddr_in es la estructura de direcciones para IPv4.
     * memset a 0 primero para limpiar el padding que podria haber.
     */
    struct sockaddr_in direccion;
    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family      = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;  /* escuchar en todas las interfaces de red */
    direccion.sin_port        = htons(puerto); /* htons: host to network short (byte order) */

    /*
     * htons() convierte el numero de puerto del orden de bytes del host
     * (little-endian en x86) al orden de bytes de la red (big-endian).
     * Sin esta conversion, el puerto en el paquete de red seria incorrecto.
     */

    /* --- bind: asociar el socket a la direccion y puerto --- */
    if (bind(server_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Error en bind");
        close(server_fd);
        logger_close();
        return 1;
    }

    /*
     * --- listen: poner el socket en modo "escucha" ---
     * El segundo parametro (128) es el "backlog": cuantas conexiones
     * puede encolar el SO mientras el proceso esta ocupado en accept().
     * Si el backlog se llena, el SO rechaza nuevas conexiones.
     * 128 es el valor maximo tipico en Linux (definido en SOMAXCONN).
     */
    if (listen(server_fd, 128) < 0) {
        perror("Error en listen");
        close(server_fd);
        logger_close();
        return 1;
    }

    log_info("Servidor listo. Esperando conexiones en puerto %d...", puerto);

    /*
     * --- Bucle principal de aceptacion ---
     * accept() BLOQUEA el hilo principal hasta que llegue una conexion.
     * Cuando llega, retorna un NUEVO file descriptor (client_fd) que
     * representa esa conexion especifica. El server_fd sigue abierto
     * para seguir aceptando mas conexiones.
     */
    while (1) {
        struct sockaddr_in addr_cliente;
        socklen_t len_addr = sizeof(addr_cliente);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&addr_cliente,
                               &len_addr);
        if (client_fd < 0) {
            log_error("Error en accept, continuando...");
            continue;  /* no terminar el servidor por un error en accept */
        }

        /* Preparar los datos para el hilo nuevo */
        DatosConexion *datos = malloc(sizeof(DatosConexion));
        if (datos == NULL) {
            log_error("Sin memoria para atender nueva conexion");
            close(client_fd);
            continue;
        }

        datos->sockfd = client_fd;

        /* inet_ntop: convierte la IP binaria de la estructura en texto legible.
         * Alternativa: inet_ntoa() → pero no es thread-safe (buffer estatico).
         * inet_ntop() es la version moderna y thread-safe. */
        inet_ntop(AF_INET, &addr_cliente.sin_addr,
                  datos->ip_cliente, INET_ADDRSTRLEN);

        /* Crear el hilo que atendra esta conexion */
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, atender_conexion, datos) != 0) {
            log_error("Error al crear hilo para %s", datos->ip_cliente);
            free(datos);
            close(client_fd);
            continue;
        }

        /*
         * pthread_detach: "desenganchamos" el hilo del hilo principal.
         * Esto significa que cuando el hilo termine, el SO libera sus
         * recursos automaticamente. No necesitamos llamar pthread_join().
         * Si NO hicieramos detach y tampoco join, habria una fuga de
         * recursos (los recursos del hilo nunca se liberarian).
         */
        pthread_detach(hilo);
    }

    /* Este codigo no se alcanza en operacion normal, pero es buena practica */
    close(server_fd);
    logger_close();
    return 0;
}
