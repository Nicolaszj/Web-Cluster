/*
 * cache.c — Módulo de caché persistente en disco con TTL
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 * Rama: feature/aws-cache
 *
 * Estrategia de almacenamiento:
 *   Por cada URI cacheada se crean DOS archivos en cache_dir:
 *
 *   1. <nombre>        — La respuesta HTTP completa (headers + body)
 *   2. <nombre>.meta   — Una sola línea con el timestamp UNIX de
 *                        cuando se almacenó (time_t como texto)
 *
 *   Ejemplo para "/caso1/index.html":
 *     ./cache/cache__caso1_index.html
 *     ./cache/cache__caso1_index.html.meta
 *
 * Solo se usan: stdio.h, stdlib.h, string.h, time.h,
 *               sys/stat.h, sys/types.h  (todo POSIX estándar)
 */

#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* ---------------------------------------------------------- */
/* Estado interno del módulo                                   */
/* ---------------------------------------------------------- */

/* El directorio ocupa hasta 255 chars; el nombre del archivo
 * ocupa el resto. Ambos juntos no superan CACHE_MAX_PATH. */
#define CACHE_DIR_MAX  256
#define CACHE_NAME_MAX (CACHE_MAX_PATH - CACHE_DIR_MAX - 8)  /* -8 por '/', '.meta' y '\0' */

static char g_cache_dir[CACHE_DIR_MAX] = "./cache";
static int  g_inicializado = 0;

/* ---------------------------------------------------------- */
/* Utilidades internas                                         */
/* ---------------------------------------------------------- */

/*
 * cache_uri_a_nombre
 * Convierte una URI en un nombre de archivo seguro para el disco.
 *
 * Reglas:
 *   - Se antepone el prefijo "cache_" para evitar colisiones
 *   - Cada '/' se reemplaza por '_'
 *   - Solo se permiten [a-zA-Z0-9._-]; el resto se reemplaza por '_'
 *
 * Ejemplo: "/caso1/index.html" → "cache__caso1_index.html"
 */
void cache_uri_a_nombre(const char *uri, char *out_buf)
{
    if (!uri || !out_buf) return;

    /* Prefijo fijo para identificar archivos de caché */
    strcpy(out_buf, "cache_");
    size_t offset = strlen(out_buf);

    for (size_t i = 0; uri[i] != '\0' && offset < CACHE_NAME_MAX - 1; i++) {
        char c = uri[i];
        if (c == '/') {
            out_buf[offset++] = '_';
        } else if ((c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') ||
                   c == '.' || c == '-') {
            out_buf[offset++] = c;
        } else {
            out_buf[offset++] = '_';
        }
    }
    out_buf[offset] = '\0';
}

/*
 * Construye la ruta completa al archivo de caché para una URI.
 * Ejemplo: uri="/caso1/index.html" → "./cache/cache__caso1_index.html"
 */
static void construir_ruta_cache(const char *uri, char *ruta_buf)
{
    char nombre[CACHE_NAME_MAX];
    cache_uri_a_nombre(uri, nombre);
    snprintf(ruta_buf, CACHE_MAX_PATH, "%s/%s", g_cache_dir, nombre);
}

/*
 * Construye la ruta al archivo .meta para una URI.
 * Ejemplo: uri="/caso1/index.html" → "./cache/cache__caso1_index.html.meta"
 */
static void construir_ruta_meta(const char *uri, char *ruta_buf)
{
    char nombre[CACHE_NAME_MAX];
    cache_uri_a_nombre(uri, nombre);
    snprintf(ruta_buf, CACHE_MAX_PATH, "%s/%s.meta", g_cache_dir, nombre);
}

/* ---------------------------------------------------------- */
/* Inicialización                                              */
/* ---------------------------------------------------------- */

int cache_init(const char *cache_dir)
{
    if (!cache_dir || cache_dir[0] == '\0') {
        fprintf(stderr, "[CACHE] Error: directorio de caché no especificado\n");
        return -1;
    }

    /* Copiar la ruta del directorio */
    strncpy(g_cache_dir, cache_dir, CACHE_DIR_MAX - 1);
    g_cache_dir[CACHE_DIR_MAX - 1] = '\0';

    /* Crear el directorio si no existe (permisos 0755) */
    if (mkdir(g_cache_dir, 0755) != 0) {
        if (errno != EEXIST) {
            perror("[CACHE] Error al crear directorio de caché");
            return -1;
        }
        /* EEXIST está bien — el directorio ya existía */
    }

    g_inicializado = 1;
    printf("[CACHE] Inicializado. Directorio: %s\n", g_cache_dir);
    return 0;
}

void cache_destroy(void)
{
    g_inicializado = 0;
    /* Aquí podrían ir limpiezas futuras (ej: liberar estructuras en RAM) */
}

/* ---------------------------------------------------------- */
/* cache_lookup — Verificar HIT o MISS                        */
/* ---------------------------------------------------------- */

int cache_lookup(const char *uri, int ttl_segundos)
{
    if (!g_inicializado || !uri) return CACHE_MISS;

    char ruta_cache[CACHE_MAX_PATH];
    char ruta_meta[CACHE_MAX_PATH];

    construir_ruta_cache(uri, ruta_cache);
    construir_ruta_meta(uri, ruta_meta);

    /* 1. Verificar que el archivo de caché existe */
    FILE *f_cache = fopen(ruta_cache, "rb");
    if (!f_cache) {
        /* Archivo no encontrado → MISS */
        return CACHE_MISS;
    }
    fclose(f_cache);

    /* 2. Leer el timestamp del archivo .meta */
    FILE *f_meta = fopen(ruta_meta, "r");
    if (!f_meta) {
        /* .meta ausente → tratar como MISS (datos inconsistentes) */
        return CACHE_MISS;
    }

    time_t timestamp_almacenado = 0;
    if (fscanf(f_meta, "%ld", (long *)&timestamp_almacenado) != 1) {
        fclose(f_meta);
        return CACHE_MISS;
    }
    fclose(f_meta);

    /* 3. Comparar con el tiempo actual */
    time_t ahora = time(NULL);
    double segundos_transcurridos = difftime(ahora, timestamp_almacenado);

    if (segundos_transcurridos > (double)ttl_segundos) {
        /* TTL expirado → MISS forzado */
        printf("[CACHE] MISS (TTL expirado, %.0fs > %ds) para: %s\n",
               segundos_transcurridos, ttl_segundos, uri);
        return CACHE_MISS;
    }

    /* 4. HIT: el recurso existe y está dentro del TTL */
    printf("[CACHE HIT] URI: %s (edad: %.0fs, TTL: %ds)\n",
           uri, segundos_transcurridos, ttl_segundos);
    return CACHE_HIT;
}

/* ---------------------------------------------------------- */
/* cache_store — Guardar respuesta en disco                   */
/* ---------------------------------------------------------- */

int cache_store(const char *uri, const char *data, size_t size)
{
    if (!g_inicializado || !uri || !data || size == 0) return -1;

    char ruta_cache[CACHE_MAX_PATH];
    char ruta_meta[CACHE_MAX_PATH];

    construir_ruta_cache(uri, ruta_cache);
    construir_ruta_meta(uri, ruta_meta);

    /* 1. Escribir la respuesta completa en el archivo de caché */
    FILE *f_cache = fopen(ruta_cache, "wb");
    if (!f_cache) {
        perror("[CACHE] Error al crear archivo de caché");
        return -1;
    }

    size_t escritos = fwrite(data, 1, size, f_cache);
    fclose(f_cache);

    if (escritos != size) {
        fprintf(stderr, "[CACHE] Error: escritura incompleta (%zu de %zu bytes)\n",
                escritos, size);
        return -1;
    }

    /* 2. Escribir el timestamp actual en el archivo .meta */
    FILE *f_meta = fopen(ruta_meta, "w");
    if (!f_meta) {
        perror("[CACHE] Error al crear archivo .meta");
        return -1;
    }

    time_t ahora = time(NULL);
    fprintf(f_meta, "%ld\n", (long)ahora);
    fclose(f_meta);

    printf("[CACHE] ALMACENADO: %s (%zu bytes) en %s\n", uri, size, ruta_cache);
    return 0;
}

/* ---------------------------------------------------------- */
/* cache_load — Leer recurso desde disco                      */
/* ---------------------------------------------------------- */

char *cache_load(const char *uri, size_t *size)
{
    if (!g_inicializado || !uri || !size) return NULL;

    char ruta_cache[CACHE_MAX_PATH];
    construir_ruta_cache(uri, ruta_cache);

    FILE *f = fopen(ruta_cache, "rb");
    if (!f) {
        fprintf(stderr, "[CACHE] No se pudo abrir: %s\n", ruta_cache);
        return NULL;
    }

    /* Obtener tamaño del archivo */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    if (file_size <= 0) {
        fclose(f);
        return NULL;
    }

    /* Reservar buffer en heap (el llamador debe hacer free()) */
    char *buffer = (char *)malloc((size_t)file_size);
    if (!buffer) {
        fprintf(stderr, "[CACHE] Error: malloc falló para %ld bytes\n", file_size);
        fclose(f);
        return NULL;
    }

    size_t leidos = fread(buffer, 1, (size_t)file_size, f);
    fclose(f);

    if (leidos != (size_t)file_size) {
        fprintf(stderr, "[CACHE] Error: lectura incompleta (%zu de %ld bytes)\n",
                leidos, file_size);
        free(buffer);
        return NULL;
    }

    *size = leidos;
    return buffer;
}
