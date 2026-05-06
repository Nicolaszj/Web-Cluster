/*
 * cache.h - Gestion de cache persistente en disco
 */

#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

/*
 * Inicializa el sistema de cache (crea directorio si no existe).
 */
void cache_init(void);

/*
 * Busca un recurso en el cache.
 * Retorna 0 si es un HIT (y llena buffer), -1 si es MISS o expiró.
 */
int cache_buscar(const char *uri, int ttl, char *buffer, size_t *size);

/*
 * Guarda un recurso en el cache.
 */
void cache_guardar(const char *uri, const char *datos, size_t size);

#endif
