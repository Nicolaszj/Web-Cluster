/*
 * logger.c - Implementacion del sistema de registro del PIBL (Proxy)
 *
 * Se utiliza un mutex para asegurar que los hilos no mezclen sus lineas
 * al escribir en la terminal o en el archivo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include "logger.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_fp = NULL;

static void get_timestamp(char *buf, int size)
{
    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", t);
}

int logger_init(const char *log_path)
{
    if (log_path != NULL) {
        log_fp = fopen(log_path, "a");
        if (log_fp == NULL) {
            perror("logger_init: no se pudo abrir el archivo de log");
            return -1;
        }
    }
    return 0;
}

void logger_close(void)
{
    if (log_fp != NULL) {
        fclose(log_fp);
        log_fp = NULL;
    }
}

void log_info(const char *fmt, ...)
{
    char ts[32];
    get_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);
    printf("[%s] [INFO] ", ts);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);

    if (log_fp != NULL) {
        fprintf(log_fp, "[%s] [INFO] ", ts);
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args);
        va_end(args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_error(const char *fmt, ...)
{
    char ts[32];
    get_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);
    fprintf(stderr, "[%s] [ERROR] ", ts);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);

    if (log_fp != NULL) {
        fprintf(log_fp, "[%s] [ERROR] ", ts);
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args);
        va_end(args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_request(const char *client_ip, const char *method,
                 const char *uri, int status_code)
{
    char ts[32];
    get_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);
    printf("[%s] %s \"%s %s\" %d\n", ts, client_ip, method, uri, status_code);
    fflush(stdout);

    if (log_fp != NULL) {
        fprintf(log_fp, "[%s] %s \"%s %s\" %d\n", ts, client_ip, method, uri, status_code);
        fflush(log_fp);
    }
    pthread_mutex_unlock(&log_mutex);
}
