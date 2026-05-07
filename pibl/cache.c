/*
 * cache.c — Módulo de caché persistente en disco con TTL y concurrencia
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 */

#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

/* ------------------------------------------------------------------ */
/* Constantes internas                                                 */
/* ------------------------------------------------------------------ */

/* Tamaño máximo del directorio de caché */
#define CACHE_DIR_MAX   256

/*
 * El nombre del archivo en disco no puede superar este tamaño:
 * CACHE_MAX_PATH - longitud_dir - 1 (slash) - 5 (.meta) - 1 (nul)
 */
#define CACHE_NOMBRE_MAX  (CACHE_MAX_PATH - CACHE_DIR_MAX - 7)

/* ------------------------------------------------------------------ */
/* Estado global del módulo                                            */
/* ------------------------------------------------------------------ */

static char         g_cache_dir[CACHE_DIR_MAX] = "./cache";
static int          g_ttl_segundos             = 60;
static int          g_inicializado             = 0;

/* Índice en memoria: arreglo estático de entradas */
static CacheEntrada g_indice[CACHE_MAX_ENTRADAS];

/* Mutex global que protege el índice completo */
static pthread_mutex_t g_mutex_indice = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------------------------------------------ */
/* Utilidades internas                                                 */
/* ------------------------------------------------------------------ */

/*
 * sanitizar_componente
 * Copia 'src' a 'dst' reemplazando caracteres no seguros por '_'.
 * Solo deja pasar: [a-zA-Z0-9], '.', '-'.
 * Escribe como máximo max_dst-1 caracteres + terminador nulo.
 */
static void sanitizar_componente(const char *src, char *dst, size_t max_dst)
{
    size_t i = 0;
    while (src[i] != '\0' && i < max_dst - 1) {
        char c = src[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-') {
            dst[i] = c;
        } else {
            dst[i] = '_';
        }
        i++;
    }
    dst[i] = '\0';
}

/*
 * cache_construir_clave  (también expuesta en .h para testing)
 * Construye la clave canónica: "METODO_host_path[_query]"
 * Los componentes se sanitizan individualmente.
 */
void cache_construir_clave(const char *metodo, const char *host,
                           const char *path,   const char *query,
                           char *out_buf)
{
    if (!metodo || !host || !path || !query || !out_buf) {
        if (out_buf) out_buf[0] = '\0';
        return;
    }

    /* Tamaños acotados para que la concatenación quepa en CACHE_MAX_CLAVE:
     * 16 + 1 + 64 + 1 + 300 + 1 + 120 + 1('\0') = 504 < 512 */
    char s_metodo[16];
    char s_host  [64];
    char s_path  [300];
    char s_query [120];

    sanitizar_componente(metodo, s_metodo, sizeof(s_metodo));
    sanitizar_componente(host,   s_host,   sizeof(s_host));
    sanitizar_componente(path,   s_path,   sizeof(s_path));
    sanitizar_componente(query,  s_query,  sizeof(s_query));

    if (query[0] != '\0') {
        snprintf(out_buf, CACHE_MAX_CLAVE, "%s_%s_%s_%s",
                 s_metodo, s_host, s_path, s_query);
    } else {
        snprintf(out_buf, CACHE_MAX_CLAVE, "%s_%s_%s",
                 s_metodo, s_host, s_path);
    }
}

/*
 * construir_ruta_datos
 * Ruta al archivo de datos: "<cache_dir>/<clave>"
 */
static void construir_ruta_datos(const char *clave, char *ruta_buf)
{
    snprintf(ruta_buf, CACHE_MAX_PATH, "%s/%s", g_cache_dir, clave);
}

/*
 * construir_ruta_meta
 * Ruta al archivo de metadatos: "<cache_dir>/<clave>.meta"
 */
static void construir_ruta_meta(const char *clave, char *ruta_buf)
{
    snprintf(ruta_buf, CACHE_MAX_PATH, "%s/%s.meta", g_cache_dir, clave);
}

/*
 * leer_timestamp_meta
 * Lee el timestamp UNIX del archivo .meta.
 * Retorna el timestamp, o 0 si no se pudo leer.
 */
static time_t leer_timestamp_meta(const char *ruta_meta)
{
    FILE *f = fopen(ruta_meta, "r");
    if (!f) return 0;

    time_t ts = 0;
    if (fscanf(f, "%ld", (long *)&ts) != 1) ts = 0;
    fclose(f);
    return ts;
}

/*
 * invalidar_entrada
 * Borra los archivos del disco y libera el slot del índice.
 * PRECONDICIÓN: llamar con g_mutex_indice adquirido.
 */
static void invalidar_entrada(CacheEntrada *entrada)
{
    char ruta_datos[CACHE_MAX_PATH];
    char ruta_meta [CACHE_MAX_PATH];

    construir_ruta_datos(entrada->clave, ruta_datos);
    construir_ruta_meta (entrada->clave, ruta_meta);

    remove(ruta_datos);
    remove(ruta_meta);

    entrada->valida = 0;
    entrada->en_escritura = 0;
    entrada->clave[0] = '\0';
    entrada->ruta[0]  = '\0';
}

/*
 * buscar_entrada
 * Busca una clave en el índice. Retorna el índice del slot o -1.
 * PRECONDICIÓN: llamar con g_mutex_indice adquirido.
 */
static int buscar_entrada(const char *clave)
{
    for (int i = 0; i < CACHE_MAX_ENTRADAS; i++) {
        if (g_indice[i].valida &&
            strcmp(g_indice[i].clave, clave) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * slot_libre
 * Busca el primer slot libre en el índice. Retorna el índice o -1.
 * PRECONDICIÓN: llamar con g_mutex_indice adquirido.
 */
static int slot_libre(void)
{
    for (int i = 0; i < CACHE_MAX_ENTRADAS; i++) {
        if (!g_indice[i].valida) return i;
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/* Inicialización y destrucción                                        */
/* ------------------------------------------------------------------ */

int cache_init(const char *cache_dir, int ttl_segundos)
{
    if (!cache_dir || cache_dir[0] == '\0') {
        fprintf(stderr, "[CACHE] Error: directorio no especificado\n");
        return -1;
    }

    strncpy(g_cache_dir, cache_dir, CACHE_DIR_MAX - 1);
    g_cache_dir[CACHE_DIR_MAX - 1] = '\0';
    g_ttl_segundos = (ttl_segundos > 0) ? ttl_segundos : 60;

    /* Crear directorio si no existe */
    if (mkdir(g_cache_dir, 0755) != 0 && errno != EEXIST) {
        perror("[CACHE] Error al crear directorio");
        return -1;
    }

    /* Inicializar índice en memoria */
    pthread_mutex_lock(&g_mutex_indice);
    for (int i = 0; i < CACHE_MAX_ENTRADAS; i++) {
        g_indice[i].valida       = 0;
        g_indice[i].en_escritura = 0;
        g_indice[i].clave[0]     = '\0';
        g_indice[i].ruta[0]      = '\0';
        pthread_cond_init(&g_indice[i].cond_escritura, NULL);
    }
    pthread_mutex_unlock(&g_mutex_indice);

    g_inicializado = 1;
    printf("[CACHE] Inicializado. Dir: %s | TTL: %ds | Slots: %d\n",
           g_cache_dir, g_ttl_segundos, CACHE_MAX_ENTRADAS);
    return 0;
}

void cache_destroy(void)
{
    if (!g_inicializado) return;

    pthread_mutex_lock(&g_mutex_indice);
    for (int i = 0; i < CACHE_MAX_ENTRADAS; i++) {
        pthread_cond_destroy(&g_indice[i].cond_escritura);
    }
    pthread_mutex_unlock(&g_mutex_indice);

    pthread_mutex_destroy(&g_mutex_indice);
    g_inicializado = 0;
}

/* ------------------------------------------------------------------ */
/* Política de cacheado                                                */
/* ------------------------------------------------------------------ */

int cache_es_cacheable(const char *metodo, int codigo_respuesta)
{
    if (!metodo) return 0;
    /* Solo GET con respuesta exitosa 200 */
    return (strcmp(metodo, "GET") == 0 && codigo_respuesta == 200) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* cache_lookup — Consulta con TTL y expiración en disco               */
/* ------------------------------------------------------------------ */

CacheResultado cache_lookup(const char *metodo, const char *host,
                            const char *path,   const char *query)
{
    if (!g_inicializado || !metodo || !host || !path || !query)
        return CACHE_MISS;

    /* Construir clave canónica */
    char clave[CACHE_MAX_CLAVE];
    cache_construir_clave(metodo, host, path, query, clave);

    pthread_mutex_lock(&g_mutex_indice);

    int idx = buscar_entrada(clave);
    if (idx == -1) {
        pthread_mutex_unlock(&g_mutex_indice);
        printf("[CACHE MISS] %s %s%s%s%s\n",
               metodo, host, path,
               query[0] ? "?" : "", query);
        return CACHE_MISS;
    }

    /* Entrada encontrada. Verificar TTL leyendo el .meta desde disco */
    char ruta_meta[CACHE_MAX_PATH];
    construir_ruta_meta(clave, ruta_meta);
    time_t ts = leer_timestamp_meta(ruta_meta);

    if (ts == 0) {
        /* .meta corrupto o ausente → invalidar */
        invalidar_entrada(&g_indice[idx]);
        pthread_mutex_unlock(&g_mutex_indice);
        printf("[CACHE MISS] Meta ausente, entrada invalidada: %s\n", clave);
        return CACHE_MISS;
    }

    double edad = difftime(time(NULL), ts);
    if (edad > (double)g_ttl_segundos) {
        /* TTL expirado: invalidar entrada y borrar archivos del disco */
        invalidar_entrada(&g_indice[idx]);
        pthread_mutex_unlock(&g_mutex_indice);
        printf("[CACHE EXPIRED] %.0fs > %ds TTL. Clave: %s\n",
               edad, g_ttl_segundos, clave);
        return CACHE_EXPIRED;
    }

    /* HIT válido */
    pthread_mutex_unlock(&g_mutex_indice);
    printf("[CACHE HIT] edad=%.0fs TTL=%ds clave=%s\n",
           edad, g_ttl_segundos, clave);
    return CACHE_HIT;
}

/* ------------------------------------------------------------------ */
/* cache_store — Guardar en disco e índice, con control de concurrencia */
/* ------------------------------------------------------------------ */

int cache_store(const char *metodo, const char *host,
                const char *path,   const char *query,
                const char *data,   size_t size)
{
    if (!g_inicializado || !metodo || !host || !path || !query ||
        !data || size == 0)
        return -1;

    /* Construir clave canónica */
    char clave[CACHE_MAX_CLAVE];
    cache_construir_clave(metodo, host, path, query, clave);

    pthread_mutex_lock(&g_mutex_indice);

    int idx = buscar_entrada(clave);

    /* ── Control de doble-escritura ─────────────────────────────────
     * Si otro hilo ya está escribiendo este mismo recurso (en_escritura == 1),
     * esperamos en la condvar. Cuando ese hilo termine, señalará la condvar
     * y nosotros chequeamos: si el recurso ya quedó válido, no escribimos
     * de nuevo (el trabajo ya está hecho).
     */
    if (idx != -1 && g_indice[idx].en_escritura) {
        while (g_indice[idx].en_escritura) {
            pthread_cond_wait(&g_indice[idx].cond_escritura,
                              &g_mutex_indice);
        }
        /* El primer hilo ya escribió. Si la entrada quedó válida, listo. */
        if (g_indice[idx].valida) {
            pthread_mutex_unlock(&g_mutex_indice);
            printf("[CACHE] Escritura evitada (otro hilo ya almacenó): %s\n",
                   clave);
            return 0;
        }
    }

    /* Buscar o crear slot */
    if (idx == -1) {
        idx = slot_libre();
        if (idx == -1) {
            pthread_mutex_unlock(&g_mutex_indice);
            fprintf(stderr, "[CACHE] Índice lleno, no se puede almacenar: %s\n",
                    clave);
            return -1;
        }
        strncpy(g_indice[idx].clave, clave, CACHE_MAX_CLAVE - 1);
        g_indice[idx].clave[CACHE_MAX_CLAVE - 1] = '\0';
    }

    /* Marcar como "en escritura" antes de soltar el mutex */
    g_indice[idx].en_escritura = 1;
    g_indice[idx].valida       = 0;   /* aún no está listo para lectura */

    pthread_mutex_unlock(&g_mutex_indice);

    /* ── Escritura en disco (fuera del mutex para no bloquear el índice) */
    char ruta_datos[CACHE_MAX_PATH];
    char ruta_meta [CACHE_MAX_PATH];
    construir_ruta_datos(clave, ruta_datos);
    construir_ruta_meta (clave, ruta_meta);

    int exito = 1;

    /* Escribir datos */
    FILE *f = fopen(ruta_datos, "wb");
    if (!f) {
        perror("[CACHE] Error al crear archivo de datos");
        exito = 0;
    } else {
        size_t escritos = fwrite(data, 1, size, f);
        fclose(f);
        if (escritos != size) {
            fprintf(stderr, "[CACHE] Escritura incompleta (%zu/%zu bytes)\n",
                    escritos, size);
            exito = 0;
        }
    }

    /* Escribir .meta con timestamp actual */
    if (exito) {
        FILE *fm = fopen(ruta_meta, "w");
        if (!fm) {
            perror("[CACHE] Error al crear .meta");
            exito = 0;
        } else {
            fprintf(fm, "%ld\n", (long)time(NULL));
            fclose(fm);
        }
    }

    /* ── Actualizar índice y notificar a hilos en espera */
    pthread_mutex_lock(&g_mutex_indice);

    if (exito) {
        strncpy(g_indice[idx].ruta, ruta_datos, CACHE_MAX_PATH - 1);
        g_indice[idx].ruta[CACHE_MAX_PATH - 1] = '\0';
        g_indice[idx].valida = 1;
        printf("[CACHE STORE] %zu bytes → %s\n", size, ruta_datos);
    } else {
        /* Escritura fallida: limpiar archivos parciales */
        remove(ruta_datos);
        remove(ruta_meta);
        g_indice[idx].valida = 0;
    }

    g_indice[idx].en_escritura = 0;
    pthread_cond_broadcast(&g_indice[idx].cond_escritura);

    pthread_mutex_unlock(&g_mutex_indice);

    return exito ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* cache_load — Leer recurso desde disco                               */
/* ------------------------------------------------------------------ */

char *cache_load(const char *metodo, const char *host,
                 const char *path,   const char *query,
                 size_t *size)
{
    if (!g_inicializado || !metodo || !host || !path || !query || !size)
        return NULL;

    char clave[CACHE_MAX_CLAVE];
    cache_construir_clave(metodo, host, path, query, clave);

    /* Verificar que la entrada existe en el índice */
    pthread_mutex_lock(&g_mutex_indice);
    int idx = buscar_entrada(clave);
    if (idx == -1 || !g_indice[idx].valida) {
        pthread_mutex_unlock(&g_mutex_indice);
        fprintf(stderr, "[CACHE] load: entrada no encontrada para %s\n", clave);
        return NULL;
    }
    char ruta[CACHE_MAX_PATH];
    strncpy(ruta, g_indice[idx].ruta, CACHE_MAX_PATH - 1);
    ruta[CACHE_MAX_PATH - 1] = '\0';
    pthread_mutex_unlock(&g_mutex_indice);

    /* Leer archivo desde disco */
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        fprintf(stderr, "[CACHE] load: no se pudo abrir %s\n", ruta);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    if (file_size <= 0) {
        fclose(f);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)file_size);
    if (!buf) {
        fprintf(stderr, "[CACHE] load: malloc falló (%ld bytes)\n", file_size);
        fclose(f);
        return NULL;
    }

    size_t leidos = fread(buf, 1, (size_t)file_size, f);
    fclose(f);

    if (leidos != (size_t)file_size) {
        fprintf(stderr, "[CACHE] load: lectura incompleta (%zu/%ld)\n",
                leidos, file_size);
        free(buf);
        return NULL;
    }

    *size = leidos;
    return buf;
}
