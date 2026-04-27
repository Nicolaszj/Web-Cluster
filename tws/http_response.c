/*
 * http_response.c - Construccion y envio de respuestas HTTP
 *
 * Este modulo resuelve:
 *   - Traducir una URI como "/pagina.html" a una ruta real del sistema
 *     de archivos como "/var/www/html/pagina.html"
 *   - Leer el archivo y enviarlo por el socket
 *   - Construir los headers HTTP correctos (Content-Type, Content-Length)
 *   - Manejar los errores 400 y 404
 *
 * Sobre envio de archivos grandes (HU-E3-11):
 * No se puede leer todo el archivo en memoria porque en el caso 3 son ~1MB
 * y en produccion podrian ser mucho mas. Se usa un buffer de 8KB y se envia
 * en trozos (chunks). Esta es la tecnica estandar en servidores web.
 *
 * Alternativa descartada: sendfile() de Linux.
 *   sendfile() es una syscall que copia directamente del buffer del kernel
 *   al socket sin pasar por espacio de usuario (cero copias). Es mucho mas
 *   eficiente, pero es especifica de Linux y el codigo perderia portabilidad.
 *   Para este proyecto, read()+send() en bucle es suficiente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "http_response.h"
#include "mime.h"
#include "logger.h"

/* Variable global definida aqui, declarada como extern en http_response.h */
char g_doc_root[512];

/* Tamanio del buffer para enviar el contenido de archivos */
#define CHUNK_SIZE 8192


/*
 * Construye el string de fecha en formato RFC 1123, que es el que HTTP exige.
 * Ejemplo: "Thu, 01 Jan 1970 00:00:00 GMT"
 * Se usa GMT porque HTTP especifica fechas siempre en UTC.
 */
static void obtener_fecha_http(char *buf, int size)
{
    time_t ahora = time(NULL);
    struct tm *gmt = gmtime(&ahora);
    strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", gmt);
}


/*
 * Envia todos los bytes del buffer al socket.
 * Por que este bucle? send() puede enviar menos bytes de los pedidos
 * si el buffer del kernel esta lleno (congestion TCP). Sin el bucle,
 * los datos quedarian truncados. Esto es especialmente importante con
 * archivos grandes del caso 3 y 4.
 * Retorna 0 si envio todo, -1 si el socket fallo en el camino.
 */
static int enviar_todo(int sockfd, const char *buf, int len)
{
    int enviados = 0;
    while (enviados < len) {
        int n = send(sockfd, buf + enviados, len - enviados, 0);
        if (n <= 0) return -1;
        enviados += n;
    }
    return 0;
}


void enviar_error(int sockfd, int codigo, const char *mensaje)
{
    char fecha[64];
    char body[256];
    char headers[512];

    obtener_fecha_http(fecha, sizeof(fecha));

    /* Pagina de error minima en HTML */
    snprintf(body, sizeof(body),
             "<html><head><title>%d %s</title></head>"
             "<body><h1>%d %s</h1></body></html>\r\n",
             codigo, mensaje, codigo, mensaje);

    int body_len = strlen(body);

    snprintf(headers, sizeof(headers),
             "HTTP/1.1 %d %s\r\n"
             "Date: %s\r\n"
             "Server: TWS/1.0\r\n"
             "Content-Type: text/html; charset=utf-8\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n",
             codigo, mensaje, fecha, body_len);

    enviar_todo(sockfd, headers, strlen(headers));
    enviar_todo(sockfd, body, body_len);
}


/*
 * Verifica que la URI no intente salir del DocumentRoot con "../".
 * Ejemplo de ataque: GET /../../../etc/passwd HTTP/1.1
 * Si lo permitieramos, el atacante podria leer cualquier archivo del sistema.
 * Esto es una vulnerabilidad de "Path Traversal" (OWASP A01).
 */
static int uri_es_segura(const char *uri)
{
    /* Rechazar cualquier URI que contenga ".." */
    if (strstr(uri, "..") != NULL) return 0;
    /* Rechazar URIs con dobles barras que podrian confundir la resolucion */
    if (strstr(uri, "//") != NULL) return 0;
    return 1;
}


/*
 * Convierte una URI HTTP en la ruta real del archivo en disco.
 * "/" → "<docroot>/index.html"
 * "/pagina.html" → "<docroot>/pagina.html"
 */
static void resolver_ruta(const char *uri, char *ruta, int ruta_size)
{
    if (strcmp(uri, "/") == 0) {
        snprintf(ruta, ruta_size, "%s/index.html", g_doc_root);
    } else {
        snprintf(ruta, ruta_size, "%s%s", g_doc_root, uri);
    }
}


/*
 * Maneja una peticion GET: sirve el archivo pedido.
 * Si send_body es 0, solo envia los headers (para HEAD).
 */
static void manejar_get_head(int sockfd, const PeticionHttp *p,
                              const char *client_ip, int send_body)
{
    const char *metodo_str = (p->metodo == METHOD_GET) ? "GET" : "HEAD";

    /* Paso 1: validar la URI */
    if (!uri_es_segura(p->uri)) {
        enviar_error(sockfd, 400, "Bad Request");
        log_request(client_ip, metodo_str, p->uri, 400);
        return;
    }

    /* Paso 2: construir la ruta del archivo */
    char ruta[1024];
    resolver_ruta(p->uri, ruta, sizeof(ruta));

    /*
     * Paso 3: verificar que el archivo existe y es un archivo regular.
     * stat() rellena una estructura con informacion del archivo:
     * tamanio, tipo (regular/directorio/enlace), permisos, etc.
     * S_ISREG() verifica que sea un archivo regular (no directorio ni device).
     */
    struct stat info;
    if (stat(ruta, &info) != 0 || !S_ISREG(info.st_mode)) {
        enviar_error(sockfd, 404, "Not Found");
        log_request(client_ip, metodo_str, p->uri, 404);
        return;
    }

    long file_size  = info.st_size;
    const char *mime = get_mime_type(ruta);

    char fecha[64];
    char headers[512];
    obtener_fecha_http(fecha, sizeof(fecha));

    /* Paso 4: construir y enviar los headers de respuesta.
     * Content-Length con el tamanio EXACTO en bytes es requerido (HU-E3-11).
     * Sin el, el cliente no sabe cuando termina la respuesta y se queda
     * bloqueado esperando mas datos. */
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Date: %s\r\n"
             "Server: TWS/1.0\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             fecha, mime, file_size);

    enviar_todo(sockfd, headers, strlen(headers));

    /* Para HEAD: solo headers, sin body (RFC 2616 sec. 9.4) */
    if (!send_body) {
        log_request(client_ip, "HEAD", p->uri, 200);
        return;
    }

    /* Paso 5: abrir y enviar el archivo en trozos de CHUNK_SIZE bytes.
     * O_RDONLY: abrir solo para lectura. Usar open() en vez de fopen()
     * porque trabaja directamente con file descriptors del SO, que son
     * los mismos que usan los sockets. Mas eficiente para transferencia. */
    int fd = open(ruta, O_RDONLY);
    if (fd < 0) {
        /* El stat() paso pero el open() fallo: puede ser un problema de permisos */
        log_error("No se pudo abrir %s", ruta);
        log_request(client_ip, "GET", p->uri, 500);
        return;
    }

    char chunk[CHUNK_SIZE];
    ssize_t leidos;

    /*
     * Bucle de envio: read() llena el buffer, send() lo envia.
     * read() retorna 0 cuando llega al EOF (fin del archivo).
     * El bucle interno con enviados/n maneja el envio parcial de send().
     */
    while ((leidos = read(fd, chunk, sizeof(chunk))) > 0) {
        ssize_t enviados = 0;
        while (enviados < leidos) {
            ssize_t s = send(sockfd, chunk + enviados, leidos - enviados, 0);
            if (s <= 0) {
                close(fd);
                log_request(client_ip, "GET", p->uri, 200);
                return;
            }
            enviados += s;
        }
    }

    close(fd);
    log_request(client_ip, "GET", p->uri, 200);
}


/*
 * Maneja una peticion POST.
 * El enunciado pide que el servidor reciba el body y confirme recepcion
 * con codigo 200. No se pide persistir los datos (no es una base de datos).
 */
static void manejar_post(int sockfd, const PeticionHttp *p,
                          const char *client_ip)
{
    char fecha[64];
    char headers[512];

    obtener_fecha_http(fecha, sizeof(fecha));

    /* Respuesta de confirmacion en JSON (formato simple y estandar) */
    const char *body = "{\"status\":\"received\",\"mensaje\":\"Datos procesados OK\"}";
    int body_len = strlen(body);

    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Date: %s\r\n"
             "Server: TWS/1.0\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n",
             fecha, body_len);

    enviar_todo(sockfd, headers, strlen(headers));
    enviar_todo(sockfd, body, body_len);

    /* Registrar en el log el tamanio del body recibido */
    log_info("POST %s - body recibido: %ld bytes", p->uri, p->body_len);
    log_request(client_ip, "POST", p->uri, 200);
}


void procesar_peticion(int sockfd, const PeticionHttp *peticion,
                       const char *client_ip)
{
    /* Si la peticion no se parseo bien, responder 400 */
    if (!peticion->valida) {
        enviar_error(sockfd, 400, "Bad Request");
        log_request(client_ip, "???", "/", 400);
        return;
    }

    switch (peticion->metodo) {
        case METHOD_GET:
            manejar_get_head(sockfd, peticion, client_ip, 1);
            break;

        case METHOD_HEAD:
            manejar_get_head(sockfd, peticion, client_ip, 0);
            break;

        case METHOD_POST:
            manejar_post(sockfd, peticion, client_ip);
            break;

        case METHOD_DESCONOCIDO:
        default:
            /* Metodo no soportado → 400 Bad Request
             * Nota: tecnicamente seria 501 Not Implemented, pero el
             * enunciado solo pide 200, 400 y 404. */
            enviar_error(sockfd, 400, "Bad Request");
            log_request(client_ip, "???", peticion->uri, 400);
            break;
    }
}
