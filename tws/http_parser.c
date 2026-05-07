/*
 * http_parser.c - Parseo de peticiones HTTP/1.1
 *
 * El protocolo HTTP/1.1 (RFC 2616) define que una peticion tiene esta forma:
 *
 *   METODO URI VERSION\r\n
 *   Header1: valor1\r\n
 *   Header2: valor2\r\n
 *   \r\n
 *   [body solo si POST y Content-Length > 0]
 *
 * Desafio tecnico: los sockets TCP no "saben" nada de HTTP. recv() nos da
 * bytes crudos, y esos bytes pueden llegar en cualquier cantidad por llamada.
 * No hay garantia de que recv() traiga exactamente una peticion completa.
 * Por eso necesitamos leer hasta encontrar el marcador de fin de headers.
 *
 * Estrategia de lectura elegida: byte a byte hasta \r\n\r\n.
 * Alternativa: leer chunks grandes y buscar \r\n\r\n con strstr().
 * Se eligio byte a byte por simplicidad y porque:
 *   - Los headers son pequeños (generalmente < 2KB).
 *   - La diferencia de velocidad es imperceptible en este contexto.
 *   - No hay riesgo de leer "de mas" y perder el inicio del body.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include "http_parser.h"


/*
 * Lee del socket byte a byte hasta encontrar el fin de los headers HTTP.
 * El fin se marca con la secuencia \r\n\r\n (linea en blanco).
 *
 * Por que leer hasta \r\n\r\n y no hasta EOF o '\n'?
 * Porque en HTTP los headers terminan con una linea en blanco (\r\n\r\n).
 * Leer hasta EOF bloquearia el socket esperando que el cliente cierre la
 * conexion, lo que nunca ocurre en keep-alive.
 *
 * Retorna: cantidad de bytes leidos, o -1 si hubo error/conexion cerrada.
 */
static int leer_headers_socket(int sockfd, char *buf, int bufsize)
{
    int total = 0;
    char byte;
    int n;

    while (total < bufsize - 1) {
        n = recv(sockfd, &byte, 1, 0);

        if (n < 0) return -1;   /* error de socket */
        if (n == 0) break;      /* cliente cerro la conexion */

        buf[total++] = byte;
        buf[total]   = '\0';

        /*
         * Verificar si los ultimos 4 bytes son \r\n\r\n.
         * Esta es la forma mas directa de detectar el fin de headers
         * sin tener que llamar strstr() en cada iteracion.
         */
        if (total >= 4 &&
            buf[total - 4] == '\r' &&
            buf[total - 3] == '\n' &&
            buf[total - 2] == '\r' &&
            buf[total - 1] == '\n') {
            break;
        }
    }

    return total;
}


/*
 * Parsea la primera linea de la peticion: "GET /ruta HTTP/1.1"
 * La modificamos con '\0' para separar los tres campos.
 * Retorna 0 si bien, -1 si el formato no es valido.
 */
static int parsear_linea_peticion(char *linea, PeticionHttp *p)
{
    char metodo_str[16];

    /* sscanf con limites de ancho para evitar desbordamiento de buffer.
     * %15s lee hasta 15 caracteres (deja espacio para el '\0'). */
    if (sscanf(linea, "%15s %1023s %15s", metodo_str, p->uri, p->version) != 3)
        return -1;

    /* Convertir el string del metodo al enum */
    if (strcmp(metodo_str, "GET")  == 0) p->metodo = METHOD_GET;
    else if (strcmp(metodo_str, "HEAD") == 0) p->metodo = METHOD_HEAD;
    else if (strcmp(metodo_str, "POST") == 0) p->metodo = METHOD_POST;
    else p->metodo = METHOD_DESCONOCIDO;

    /* El enunciado pide soportar HTTP/1.1. Tambien aceptamos 1.0 por
     * compatibilidad (curl --http1.0, clientes viejos). */
    if (strncmp(p->version, "HTTP/", 5) != 0)
        return -1;

    return 0;
}


/*
 * Parsea la seccion de headers de la peticion.
 * Cada linea tiene formato "Nombre: valor\r\n".
 * Modifica el buffer temporalmente con '\0' para separar nombre y valor.
 */
static void parsear_headers(char *inicio, PeticionHttp *p)
{
    char *linea     = inicio;
    char *fin_linea;

    while (*linea != '\0') {
        fin_linea = strstr(linea, "\r\n");
        if (fin_linea == NULL) break;

        /* Si la linea esta vacia, llegamos al fin de los headers */
        if (fin_linea == linea) break;

        *fin_linea = '\0';  /* terminar el string de esta linea */

        /* Separar nombre y valor en el ':' */
        char *dos_puntos = strchr(linea, ':');
        if (dos_puntos != NULL && p->num_headers < MAX_HEADERS) {
            *dos_puntos = '\0';
            char *valor = dos_puntos + 1;

            /* Saltar los espacios iniciales del valor ("Host:  ejemplo.com") */
            while (*valor == ' ' || *valor == '\t') valor++;

            /* Copiar con limite para no desbordar los buffers de HeaderHttp */
            strncpy(p->headers[p->num_headers].nombre, linea, 127);
            p->headers[p->num_headers].nombre[127] = '\0';
            strncpy(p->headers[p->num_headers].valor, valor, HEADER_VAL_MAX - 1);
            p->headers[p->num_headers].valor[HEADER_VAL_MAX - 1] = '\0';
            p->num_headers++;
        }

        /* Avanzar al inicio de la siguiente linea (saltar \r\n) */
        linea = fin_linea + 2;
    }
}


int parsear_peticion(int sockfd, PeticionHttp *peticion)
{
    /* Inicializar la estructura a ceros para que no haya basura en memoria */
    memset(peticion, 0, sizeof(PeticionHttp));

    /* Buffer en el stack para leer los headers.
     * Por que stack y no malloc? Porque el tamanio es fijo y conocido en
     * tiempo de compilacion. Malloc tendria overhead innecesario aqui. */
    char buf[REQUEST_BUF_SIZE];

    int leidos = leer_headers_socket(sockfd, buf, sizeof(buf));
    if (leidos <= 0)
        return -1;  /* socket cerrado o error antes de recibir datos */

    /* --- Paso 1: parsear la linea de peticion --- */
    char *fin_primera = strstr(buf, "\r\n");
    if (fin_primera == NULL)
        return 0;  /* datos recibidos pero sin \r\n = request invalida */

    *fin_primera = '\0';  /* aislar la primera linea */

    if (parsear_linea_peticion(buf, peticion) < 0)
        return 0;  /* formato incorrecto, peticion->valida sigue en 0 */

    /* --- Paso 2: parsear los headers --- */
    char *inicio_headers = fin_primera + 2;  /* saltar el \r\n */
    parsear_headers(inicio_headers, peticion);

    /* --- Paso 3: leer body si es POST --- */
    if (peticion->metodo == METHOD_POST) {
        const char *cl = buscar_header(peticion, "Content-Length");

        /*
         * HTTP/1.1 exige Content-Length en POST (RFC 2616 sec. 14.13).
         * Sin el, no sabemos cuantos bytes leer del body → 400.
         * Por eso peticion->valida queda en 0 aqui.
         */
        if (cl == NULL) {
            return 0;  /* POST sin Content-Length → invalida */
        }

        peticion->body_len = atol(cl);

        /* Limite de 10MB para el body. No es un limite del RFC, sino una
         * decision de diseño para no agotar la memoria del servidor con
         * una peticion maliciosa o accidental. */
        if (peticion->body_len < 0 || peticion->body_len > 10485760) {
            return 0;
        }

        if (peticion->body_len > 0) {
            /* +1 para el '\0' que añadiremos al final */
            peticion->body = malloc(peticion->body_len + 1);
            if (peticion->body == NULL)
                return 0;

            /* Leer exactamente body_len bytes del socket.
             * Un solo recv() no garantiza que lleguen todos de una vez,
             * por eso usamos un bucle acumulador. */
            long recibidos = 0;
            while (recibidos < peticion->body_len) {
                long n = recv(sockfd,
                              peticion->body + recibidos,
                              peticion->body_len - recibidos,
                              0);
                if (n <= 0) break;
                recibidos += n;
            }
            peticion->body[peticion->body_len] = '\0';
        }
    }

    peticion->valida = 1;
    return 0;
}


void liberar_peticion(PeticionHttp *peticion)
{
    if (peticion->body != NULL) {
        free(peticion->body);
        peticion->body = NULL;
    }
}


const char *buscar_header(const PeticionHttp *peticion, const char *nombre)
{
    for (int i = 0; i < peticion->num_headers; i++) {
        /* strcasecmp: "content-length" == "Content-Length" == "CONTENT-LENGTH"
         * HTTP especifica que los nombres de header no son case-sensitive. */
        if (strcasecmp(peticion->headers[i].nombre, nombre) == 0) {
            return peticion->headers[i].valor;
        }
    }
    return NULL;
}
