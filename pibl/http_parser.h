#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

/*
 * http_parser.h - Estructuras y funciones para parsear peticiones HTTP/1.1
 *
 * El parser es el corazon del servidor web: lee los bytes crudos del socket
 * y los convierte en una estructura de datos con la que podemos trabajar.
 *
 * RFC 2616 (HTTP/1.1) define el formato de una peticion asi:
 *
 *   GET /index.html HTTP/1.1\r\n       ← Linea de peticion
 *   Host: www.ejemplo.com\r\n          ← Headers (uno por linea)
 *   Accept: text/html\r\n
 *   \r\n                               ← Linea en blanco = fin de headers
 *   [body]                             ← Solo si es POST y tiene Content-Length
 */

/* Tamanios maximos. Se eligieron valores razonables:
 * - 16KB para headers: es mas que suficiente; Nginx usa 8KB por defecto.
 * - 1024 para URI: URLs mas largas son inusuales en servidores web normales.
 * - 512 para valores de header: cubre la mayoria de casos reales. */
#define REQUEST_BUF_SIZE  16384
#define URI_MAX           1024
#define HEADER_VAL_MAX    512
#define MAX_HEADERS       50


/* Los tres metodos que soporta el TWS segun el enunciado + uno para lo demas */
typedef enum {
    METHOD_GET,
    METHOD_HEAD,
    METHOD_POST,
    METHOD_DESCONOCIDO
} MetodoHttp;


/* Un header HTTP es simplemente un par nombre:valor.
 * Ejemplo: "Content-Type" → "text/html" */
typedef struct {
    char nombre[128];
    char valor[HEADER_VAL_MAX];
} HeaderHttp;


/*
 * Estructura que representa una peticion HTTP completa ya parseada.
 * Esto es lo que el handler recibe para construir la respuesta.
 */
typedef struct {
    MetodoHttp  metodo;
    char        uri[URI_MAX];
    char        version[16];           /* "HTTP/1.1", "HTTP/1.0", etc. */
    HeaderHttp  headers[MAX_HEADERS];
    int         num_headers;
    char       *body;                  /* NULL si no hay body (GET/HEAD) */
    long        body_len;
    int         valida;                /* 1 si el parseo fue exitoso */
} PeticionHttp;


/*
 * Lee una peticion HTTP del socket y la parsea en la estructura.
 * Retorna:
 *   0  → se leyeron datos (verificar peticion->valida para saber si parseo bien)
 *  -1  → error de socket o conexion cerrada antes de recibir datos
 */
int parsear_peticion(int sockfd, PeticionHttp *peticion);

/* Libera el body si fue asignado dinamicamente con malloc() */
void liberar_peticion(PeticionHttp *peticion);

/*
 * Busca el valor de un header por su nombre (no distingue mayusculas).
 * Retorna NULL si no existe el header.
 * Ejemplo: buscar_header(&req, "Content-Length") → "1024"
 */
const char *buscar_header(const PeticionHttp *peticion, const char *nombre);

#endif
