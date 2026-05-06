/*
 * http_proxy.h - Logica de forwarding y gestion de peticiones en el proxy
 */

#ifndef HTTP_PROXY_H
#define HTTP_PROXY_H

#include "config.h"

/*
 * Recibe la peticion del cliente, elige un backend, le reenvia la peticion
 * y devuelve la respuesta al cliente.
 */
void gestionar_proxy(int client_fd, const char *client_ip, ConfigProxy *config);

#endif
