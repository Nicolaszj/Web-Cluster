/*
 * config.h - Definiciones para la configuracion del Proxy (PIBL)
 */

#ifndef CONFIG_H
#define CONFIG_H

#define MAX_BACKENDS 16
#define MAX_HOST_LEN 256

typedef struct {
    char host[MAX_HOST_LEN];
    int  port;
} Backend;

typedef struct {
    int     puerto_escucha;
    Backend backends[MAX_BACKENDS];
    int     num_backends;
    int     cache_ttl;
} ConfigProxy;

/*
 * Lee el archivo de configuracion y llena la estructura ConfigProxy.
 * Retorna 0 si exito, -1 si hay error.
 */
int cargar_configuracion(const char *ruta, ConfigProxy *config);

#endif
