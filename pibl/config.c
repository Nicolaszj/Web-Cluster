/*
 * config.c — Parser del archivo de configuración del Proxy PIBL
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 * Rama: feature/aws-cache
 *
 * Solo usa: stdio.h, stdlib.h, string.h  (100% POSIX / stdlib estándar)
 *
 * Formato reconocido:
 *   # Comentario (línea ignorada)
 *   port=8080
 *   ttl=60
 *   backend=192.168.1.10:9090
 *   backend=192.168.1.11:9090
 *
 * Reglas del parser:
 *   - Las líneas vacías y las que comienzan con '#' se ignoran
 *   - Los espacios alrededor de '=' y al inicio/fin de línea se eliminan
 *   - Los campos desconocidos se ignoran (forward-compatible)
 *   - 'port' y al menos un 'backend' son obligatorios
 *   - 'ttl' es opcional; si no aparece se usa TTL_DEFAULT
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* TTL por defecto si no se especifica en el archivo */
#define TTL_DEFAULT 60

/* ---------------------------------------------------------- */
/* Utilidades internas                                         */
/* ---------------------------------------------------------- */

/**
 * Elimina espacios y '\r' al inicio y al final de una cadena.
 * Modifica la cadena en su lugar y retorna el puntero al primer
 * carácter no-espacio.
 */
static char *trim(char *s)
{
    if (!s) return s;

    /* Avanzar sobre espacios del inicio */
    while (*s && isspace((unsigned char)*s)) s++;

    /* Si quedó vacía, retornar */
    if (*s == '\0') return s;

    /* Retroceder desde el final eliminando espacios */
    char *fin = s + strlen(s) - 1;
    while (fin > s && isspace((unsigned char)*fin)) {
        *fin = '\0';
        fin--;
    }

    return s;
}

/**
 * Parsea una línea "backend=IP:PUERTO" y rellena un struct Backend.
 *
 * @param valor   Cadena con el valor (parte derecha del '='), ej: "192.168.1.10:9090"
 * @param backend Puntero al Backend a rellenar
 * @return 0 en éxito, -1 si el formato es inválido
 */
static int parsear_backend(const char *valor, Backend *backend)
{
    if (!valor || !backend) return -1;

    /* Buscar el separador ':' */
    const char *sep = strchr(valor, ':');
    if (!sep) {
        fprintf(stderr, "[CONFIG] Error: backend '%s' no tiene formato IP:PUERTO\n", valor);
        return -1;
    }

    /* Longitud de la parte IP */
    size_t ip_len = (size_t)(sep - valor);
    if (ip_len == 0 || ip_len >= CONFIG_MAX_IP_LEN) {
        fprintf(stderr, "[CONFIG] Error: IP inválida en backend '%s'\n", valor);
        return -1;
    }

    /* Copiar IP */
    strncpy(backend->ip, valor, ip_len);
    backend->ip[ip_len] = '\0';

    /* Parsear el puerto */
    char *endptr;
    long puerto = strtol(sep + 1, &endptr, 10);
    if (endptr == sep + 1 || *endptr != '\0' || puerto <= 0 || puerto > 65535) {
        fprintf(stderr, "[CONFIG] Error: puerto inválido en backend '%s'\n", valor);
        return -1;
    }

    backend->puerto = (int)puerto;
    return 0;
}

/* ---------------------------------------------------------- */
/* API pública                                                 */
/* ---------------------------------------------------------- */

int config_cargar(const char *ruta, Config *cfg)
{
    if (!ruta || !cfg) return -1;

    /* Inicializar la struct a ceros */
    memset(cfg, 0, sizeof(Config));
    cfg->ttl = TTL_DEFAULT;  /* Valor por defecto */

    FILE *f = fopen(ruta, "r");
    if (!f) {
        fprintf(stderr, "[CONFIG] Error: no se puede abrir '%s'\n", ruta);
        return -1;
    }

    char linea[CONFIG_MAX_LINE_LEN];
    int  num_linea = 0;
    int  puerto_encontrado = 0;

    while (fgets(linea, sizeof(linea), f)) {
        num_linea++;

        /* Eliminar espacios y '\r' */
        char *l = trim(linea);

        /* Ignorar líneas vacías y comentarios */
        if (*l == '\0' || *l == '#') continue;

        /* Buscar el separador '=' */
        char *sep = strchr(l, '=');
        if (!sep) {
            fprintf(stderr, "[CONFIG] Advertencia línea %d: sin '=' — ignorada: %s\n",
                    num_linea, l);
            continue;
        }

        /* Separar clave y valor */
        *sep = '\0';
        char *clave = trim(l);
        char *valor = trim(sep + 1);

        /* ---- Procesar clave ---- */
        if (strcmp(clave, "port") == 0) {
            char *endptr;
            long p = strtol(valor, &endptr, 10);
            if (endptr == valor || *endptr != '\0' || p <= 0 || p > 65535) {
                fprintf(stderr, "[CONFIG] Error línea %d: puerto inválido '%s'\n",
                        num_linea, valor);
                fclose(f);
                return -1;
            }
            cfg->puerto = (int)p;
            puerto_encontrado = 1;

        } else if (strcmp(clave, "ttl") == 0) {
            char *endptr;
            long t = strtol(valor, &endptr, 10);
            if (endptr == valor || *endptr != '\0' || t <= 0) {
                fprintf(stderr, "[CONFIG] Advertencia línea %d: TTL inválido '%s', "
                        "usando default %d s\n", num_linea, valor, TTL_DEFAULT);
                cfg->ttl = TTL_DEFAULT;
            } else {
                cfg->ttl = (int)t;
            }

        } else if (strcmp(clave, "backend") == 0) {
            if (cfg->num_backends >= CONFIG_MAX_BACKENDS) {
                fprintf(stderr, "[CONFIG] Advertencia: máximo de backends (%d) alcanzado, "
                        "línea %d ignorada\n", CONFIG_MAX_BACKENDS, num_linea);
                continue;
            }
            Backend b;
            memset(&b, 0, sizeof(b));
            if (parsear_backend(valor, &b) == 0) {
                cfg->backends[cfg->num_backends++] = b;
            } else {
                fprintf(stderr, "[CONFIG] Advertencia línea %d: backend inválido ignorado\n",
                        num_linea);
            }

        } else {
            /* Campo desconocido — ignorar sin error (forward-compatible) */
            fprintf(stderr, "[CONFIG] Advertencia línea %d: campo desconocido '%s' ignorado\n",
                    num_linea, clave);
        }
    }

    fclose(f);

    /* ---- Validar campos obligatorios ---- */
    if (!puerto_encontrado) {
        fprintf(stderr, "[CONFIG] Error: falta el campo obligatorio 'port' en '%s'\n", ruta);
        return -2;
    }

    if (cfg->num_backends == 0) {
        fprintf(stderr, "[CONFIG] Error: se necesita al menos un 'backend' en '%s'\n", ruta);
        return -2;
    }

    return 0;
}

void config_imprimir(const Config *cfg)
{
    if (!cfg) return;

    printf("[CONFIG] ===== Configuración del Proxy PIBL =====\n");
    printf("[CONFIG]   Puerto de escucha : %d\n", cfg->puerto);
    printf("[CONFIG]   TTL del caché     : %d segundos\n", cfg->ttl);
    printf("[CONFIG]   Backends (%d):\n", cfg->num_backends);
    for (int i = 0; i < cfg->num_backends; i++) {
        printf("[CONFIG]     [%d] %s:%d\n", i + 1,
               cfg->backends[i].ip,
               cfg->backends[i].puerto);
    }
    printf("[CONFIG] ==========================================\n");
}

void config_destruir(Config *cfg)
{
    if (!cfg) return;
    /*
     * Actualmente toda la memoria está en la struct (sin malloc dinámico).
     * Se limpia con memset para evitar datos residuales en stack.
     */
    memset(cfg, 0, sizeof(Config));
}
