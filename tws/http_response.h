#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

/*
 * http_response.h - Construccion y envio de respuestas HTTP (HU-E3-04, E3-05)
 *
 * Una respuesta HTTP/1.1 tiene esta estructura:
 *
 *   HTTP/1.1 200 OK\r\n               ← Status line
 *   Content-Type: text/html\r\n       ← Headers de respuesta
 *   Content-Length: 1234\r\n
 *   Connection: close\r\n
 *   \r\n                              ← Linea en blanco obligatoria
 *   <contenido del archivo>           ← Body (ausente en HEAD)
 */

#include "http_parser.h"

/*
 * Variable global con la ruta raiz de los documentos.
 * Es "extern" aqui para que otros .c puedan leerla.
 * Solo se escribe una vez en main(), antes de que arranquen los hilos,
 * por eso no necesita mutex. Despues es solo lectura concurrente.
 */
extern char g_doc_root[512];

/*
 * Procesa una peticion HTTP completa y envia la respuesta al socket.
 * Esta es la funcion central: decide que responder segun el metodo y URI.
 * client_ip se usa solo para el log.
 */
void procesar_peticion(int sockfd, const PeticionHttp *peticion,
                       const char *client_ip);

/*
 * Envia una respuesta de error al cliente.
 * Disponible como funcion publica para que server.c la use cuando
 * el parseo falla antes de llegar a procesar_peticion().
 */
void enviar_error(int sockfd, int codigo, const char *mensaje);

#endif
