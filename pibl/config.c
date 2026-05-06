/*
 * config.c - Implementacion del lector de configuracion del Proxy
 *
 * El archivo de configuracion sigue un formato simple de "clave valor":
 *   listen 8080
 *   backend 127.0.0.1 8081
 *   backend 127.0.0.1 8082
 *   cache_ttl 60
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int cargar_configuracion(const char *ruta, ConfigProxy *config)
{
    FILE *fp = fopen(ruta, "r");
    if (fp == NULL) {
        perror("Error al abrir archivo de configuracion");
        return -1;
    }

    /* Valores por defecto */
    memset(config, 0, sizeof(ConfigProxy));
    config->puerto_escucha = 8080;
    config->cache_ttl = 60;

    char linea[512];
    while (fgets(linea, sizeof(linea), fp)) {
        /* Ignorar comentarios y lineas vacias */
        if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\r' || linea[0] == ' ')
            continue;

        /* Parsear "listen <puerto>" */
        if (strncmp(linea, "listen", 6) == 0) {
            sscanf(linea, "listen %d", &config->puerto_escucha);
        }
        /* Parsear "backend <host> <puerto>" */
        else if (strncmp(linea, "backend", 7) == 0) {
            if (config->num_backends < MAX_BACKENDS) {
                Backend *b = &config->backends[config->num_backends];
                if (sscanf(linea, "backend %255s %d", b->host, &b->port) == 2) {
                    config->num_backends++;
                }
            }
        }
        /* Parsear "cache_ttl <segundos>" */
        else if (strncmp(linea, "cache_ttl", 9) == 0) {
            sscanf(linea, "cache_ttl %d", &config->cache_ttl);
        }
    }

    fclose(fp);

    /* Validacion minima */
    if (config->num_backends == 0) {
        fprintf(stderr, "Error: No se definieron backends en el archivo de configuracion.\n");
        return -1;
    }

    return 0;
}
