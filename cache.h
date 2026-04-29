#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>  /* size_t */

/* ============================================================
 * cache.h — Módulo de caché persistente en disco
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 * Rama: feature/aws-cache
 *
 * Uso básico:
 *   cache_init("./cache");
 *   int resultado = cache_lookup("/caso1/index.html", 60);
 *   if (resultado == CACHE_HIT) {
 *       size_t size;
 *       char *data = cache_load("/caso1/index.html", &size);
 *       // enviar data al cliente ...
 *       free(data);
 *   } else {
 *       // obtener respuesta del backend ...
 *       cache_store("/caso1/index.html", respuesta, respuesta_size);
 *   }
 * ============================================================ */

/* Valores de retorno de cache_lookup */
#define CACHE_HIT   0
#define CACHE_MISS  1

/* Longitud máxima de una ruta de archivo de caché */
#define CACHE_MAX_PATH 512

/* Longitud máxima de una URI */
#define CACHE_MAX_URI  1024

/* ---------------------------------------------------------- */
/* Inicialización                                              */
/* ---------------------------------------------------------- */

/**
 * Inicializa el módulo de caché.
 * Crea el directorio cache_dir si no existe.
 * Debe llamarse una sola vez al arrancar el PIBL.
 *
 * @param cache_dir  Ruta al directorio de caché (ej: "./cache")
 * @return  0 en éxito, -1 en error
 */
int cache_init(const char *cache_dir);

/* ---------------------------------------------------------- */
/* Operaciones principales                                     */
/* ---------------------------------------------------------- */

/**
 * Consulta si un recurso está en caché y su TTL es válido.
 *
 * @param uri          URI del recurso (ej: "/caso1/index.html")
 * @param ttl_segundos Tiempo de vida en segundos (leído del config)
 * @return  CACHE_HIT  si existe y no ha expirado
 *          CACHE_MISS si no existe o TTL expirado
 */
int cache_lookup(const char *uri, int ttl_segundos);

/**
 * Guarda una respuesta completa del backend en disco.
 * También escribe el archivo .meta con el timestamp actual.
 *
 * @param uri   URI del recurso
 * @param data  Puntero al buffer con la respuesta completa (headers + body)
 * @param size  Tamaño en bytes del buffer
 * @return  0 en éxito, -1 en error
 */
int cache_store(const char *uri, const char *data, size_t size);

/**
 * Carga un recurso desde caché.
 * El llamador es responsable de liberar la memoria con free().
 *
 * @param uri   URI del recurso
 * @param size  [out] Tamaño en bytes del buffer retornado
 * @return  Puntero al buffer con los datos, o NULL en error
 */
char *cache_load(const char *uri, size_t *size);

/* ---------------------------------------------------------- */
/* Utilidades internas (expuestas para testing)               */
/* ---------------------------------------------------------- */

/**
 * Convierte una URI en un nombre de archivo seguro para disco.
 * Reemplaza '/' por '_' y elimina caracteres problemáticos.
 * Ejemplo: "/caso1/index.html" → "cache__caso1_index.html"
 *
 * @param uri       URI de entrada
 * @param out_buf   Buffer de salida (tamaño mínimo: CACHE_MAX_PATH)
 */
void cache_uri_a_nombre(const char *uri, char *out_buf);

/**
 * Termina el módulo de caché (no hace nada crítico, pero
 * deja la puerta abierta para futuras limpiezas).
 */
void cache_destroy(void);

#endif /* CACHE_H */
