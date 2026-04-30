#ifndef CACHE_H
#define CACHE_H

/*
 * cache.h — Módulo de caché persistente en disco con TTL y concurrencia
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 * Rama: feature/aws-cache
 *
 * ── Arquitectura ──────────────────────────────────────────────────────
 *
 *  Índice en memoria (g_indice[]):
 *    Arreglo de CacheEntrada con la clave, ruta en disco y estado.
 *    Permite saber en O(n) si un recurso está cacheado sin abrir archivos.
 *    Protegido por g_mutex_indice (mutex global del índice).
 *
 *  Disco (cache_dir/):
 *    <clave_sanitizada>       — respuesta HTTP completa (headers + body)
 *    <clave_sanitizada>.meta  — timestamp UNIX de almacenamiento
 *
 * ── Clave de caché ────────────────────────────────────────────────────
 *
 *  La clave se construye concatenando: MÉTODO + Host + Path + Query
 *  Ejemplo: "GET_localhost__caso1_index.html"
 *  Esto garantiza que dos recursos con el mismo path pero diferente
 *  host o método no colisionen en el índice.
 *
 * ── Política de cacheado ──────────────────────────────────────────────
 *
 *  Solo se cachean respuestas GET con código HTTP 200.
 *  POST, HEAD y cualquier otro método NO se cachean.
 *  Códigos != 200 (404, 500, etc.) NO se almacenan.
 *
 * ── Concurrencia ──────────────────────────────────────────────────────
 *
 *  Un mutex global protege el índice en memoria.
 *  Cada entrada tiene un flag "en_escritura" y una condvar que evita
 *  que dos hilos escriban simultáneamente el mismo recurso.
 *
 * ── Uso típico en el proxy ────────────────────────────────────────────
 *
 *   // Al arrancar:
 *   cache_init("./cache", cfg.ttl);
 *
 *   // En cada petición:
 *   if (!cache_es_cacheable(metodo, codigo_backend)) { ... }
 *
 *   CacheResultado r = cache_lookup(metodo, host, path, query);
 *   if (r == CACHE_HIT) {
 *       size_t size;
 *       char *data = cache_load(metodo, host, path, query, &size);
 *       send(fd_cliente, data, size, 0);
 *       free(data);
 *   } else {
 *       // CACHE_MISS o CACHE_EXPIRED → ir al backend
 *       cache_store(metodo, host, path, query, respuesta, respuesta_size);
 *       send(fd_cliente, respuesta, respuesta_size, 0);
 *   }
 *
 *   // Al terminar el PIBL:
 *   cache_destroy();
 */

#include <stddef.h>   /* size_t             */
#include <pthread.h>  /* pthread_mutex_t, pthread_cond_t */

/* ------------------------------------------------------------------ */
/* Constantes                                                          */
/* ------------------------------------------------------------------ */

/** Número máximo de entradas en el índice en memoria */
#define CACHE_MAX_ENTRADAS  256

/** Longitud máxima de una clave de caché sanitizada */
#define CACHE_MAX_CLAVE     512

/** Longitud máxima de una ruta de archivo en disco */
#define CACHE_MAX_PATH      512

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

/**
 * Resultado de cache_lookup().
 * Se usa typedef enum para que el compilador fuerce
 * que solo se usen estos tres valores, no enteros arbitrarios.
 */
typedef enum {
    CACHE_HIT     = 0,  /**< Recurso en caché con TTL válido   */
    CACHE_MISS    = 1,  /**< Recurso no encontrado en caché    */
    CACHE_EXPIRED = 2   /**< Encontrado pero TTL expirado      */
} CacheResultado;

/**
 * Una entrada del índice en memoria.
 * Cada recurso cacheado ocupa exactamente un slot aquí.
 */
typedef struct {
    char   clave[CACHE_MAX_CLAVE]; /**< Clave sanitizada: "METODO_host_path_query" */
    char   ruta[CACHE_MAX_PATH];   /**< Ruta al archivo de datos en disco          */
    int    valida;                 /**< 1 = slot ocupado, 0 = slot libre            */
    int    en_escritura;           /**< 1 = un hilo está escribiendo este recurso  */
    pthread_cond_t cond_escritura; /**< Hilos esperan aquí si en_escritura == 1    */
} CacheEntrada;

/* ------------------------------------------------------------------ */
/* API pública                                                         */
/* ------------------------------------------------------------------ */

/**
 * Inicializa el módulo de caché.
 *
 * - Crea el directorio en disco si no existe.
 * - Inicializa el índice en memoria (todos los slots libres).
 * - Inicializa el mutex global y las condvars.
 * - Debe llamarse UNA SOLA VEZ al arrancar el PIBL.
 *
 * @param cache_dir    Directorio de caché (ej: "./cache")
 * @param ttl_segundos TTL global leído de config.txt
 * @return 0 en éxito, -1 en error
 */
int cache_init(const char *cache_dir, int ttl_segundos);

/**
 * Libera todos los recursos del módulo (mutex, condvars del índice).
 * Llamar justo antes de terminar el proceso PIBL.
 */
void cache_destroy(void);

/**
 * Determina si una combinación método+código es cacheable.
 * Política: solo GET con código 200.
 *
 * @param metodo           Método HTTP ("GET", "POST", …)
 * @param codigo_respuesta Código HTTP de la respuesta del backend
 * @return 1 si se debe cachear, 0 si no
 */
int cache_es_cacheable(const char *metodo, int codigo_respuesta);

/**
 * Consulta el índice y verifica el TTL del recurso.
 *
 * - Hilo-seguro: adquiere g_mutex_indice durante la búsqueda.
 * - Si el recurso expiró: lo marca como inválido en el índice
 *   y borra los archivos del disco (dato + .meta).
 * - Retorna CACHE_EXPIRED (no CACHE_MISS) para que el proxy
 *   pueda registrar el evento diferenciado en el log.
 *
 * @param metodo  Método HTTP (ej: "GET")
 * @param host    Header Host (ej: "192.168.1.1")
 * @param path    Path de la URI (ej: "/caso1/index.html")
 * @param query   Query string sin '?' (ej: "v=2"), o "" si no hay
 * @return CACHE_HIT | CACHE_MISS | CACHE_EXPIRED
 */
CacheResultado cache_lookup(const char *metodo, const char *host,
                            const char *path,   const char *query);

/**
 * Guarda la respuesta en disco y registra la entrada en el índice.
 *
 * Evita doble-escritura: si otro hilo ya está escribiendo el mismo
 * recurso (en_escritura == 1), este hilo espera en la condvar hasta
 * que el primero termine, y luego retorna sin escribir de nuevo.
 *
 * @param metodo  Método HTTP
 * @param host    Header Host
 * @param path    Path de la URI
 * @param query   Query string (o "")
 * @param data    Respuesta HTTP completa (headers + body)
 * @param size    Tamaño de data en bytes
 * @return 0 en éxito, -1 en error
 */
int cache_store(const char *metodo, const char *host,
                const char *path,   const char *query,
                const char *data,   size_t size);

/**
 * Lee el recurso desde disco y lo devuelve en un buffer en heap.
 * El llamador DEBE llamar free() sobre el puntero retornado.
 *
 * @param metodo  Método HTTP
 * @param host    Header Host
 * @param path    Path de la URI
 * @param query   Query string (o "")
 * @param size    [out] Tamaño en bytes del buffer retornado
 * @return Buffer con la respuesta completa, o NULL en error
 */
char *cache_load(const char *metodo, const char *host,
                 const char *path,   const char *query,
                 size_t *size);

/* ------------------------------------------------------------------ */
/* Utilidad expuesta para testing                                      */
/* ------------------------------------------------------------------ */

/**
 * Construye la clave canónica de caché a partir de sus componentes.
 *
 * Formato: "<METODO>_<host>_<path_sanitizado>[_<query_sanitizada>]"
 * Caracteres no alfanuméricos (excepto '.' y '-') se reemplazan por '_'.
 *
 * Ejemplos:
 *   "GET" + "localhost" + "/caso1/index.html" + ""
 *     → "GET_localhost__caso1_index.html"
 *
 *   "GET" + "10.0.0.1" + "/buscar" + "q=hola"
 *     → "GET_10.0.0.1__buscar_q_hola"
 *
 * @param metodo, host, path, query  Componentes de la clave
 * @param out_buf   Buffer de salida (tamaño mínimo: CACHE_MAX_CLAVE)
 */
void cache_construir_clave(const char *metodo, const char *host,
                           const char *path,   const char *query,
                           char *out_buf);

#endif /* CACHE_H */
