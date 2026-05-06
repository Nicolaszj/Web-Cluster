/*
 * cache.c - Implementacion de cache persistente en disco
 *
 * El cache guarda las respuestas completas (headers + body) en archivos.
 * El nombre del archivo se deriva de la URI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "cache.h"
#include "logger.h"

#define CACHE_DIR "./cache_data"

/* Auxiliar para convertir URI en nombre de archivo seguro */
static void uri_a_filename(const char *uri, char *filename, int size) {
    snprintf(filename, size, "%s/", CACHE_DIR);
    int offset = strlen(filename);
    
    for (int i = 0; uri[i] != '\0' && offset < size - 1; i++) {
        if (uri[i] == '/') filename[offset++] = '_';
        else filename[offset++] = uri[i];
    }
    filename[offset] = '\0';
}

void cache_init(void) {
    /* Crear el directorio de cache si no existe */
    #ifdef _WIN32
        mkdir(CACHE_DIR);
    #else
        mkdir(CACHE_DIR, 0755);
    #endif
}

int cache_buscar(const char *uri, int ttl, char *buffer, size_t *size) {
    char path[512];
    uri_a_filename(uri, path, sizeof(path));

    struct stat st;
    if (stat(path, &st) != 0) return -1; /* No existe */

    /* Verificar TTL */
    time_t ahora = time(NULL);
    if (difftime(ahora, st.st_mtime) > ttl) {
        log_info("Cache expirado para %s", uri);
        unlink(path); /* Borrar archivo expirado */
        return -1;
    }

    /* Leer contenido */
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return -1;

    size_t leidos = fread(buffer, 1, *size, fp);
    *size = leidos;
    fclose(fp);

    return 0;
}

void cache_guardar(const char *uri, const char *datos, size_t size) {
    char path[512];
    uri_a_filename(uri, path, sizeof(path));

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        log_error("No se pudo escribir en cache: %s", path);
        return;
    }

    fwrite(datos, 1, size, fp);
    fclose(fp);
    log_info("Recurso guardado en cache: %s", uri);
}
