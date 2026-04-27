/*
 * logger.c - Implementacion del sistema de registro del TWS
 *
 * Tecnologia elegida: pthread_mutex_t (POSIX)
 *
 * Alternativas que existen:
 *   1. No usar mutex → logs mezclados cuando hay concurrencia. Descartado.
 *   2. Un hilo dedicado solo para escribir logs (productor-consumidor con cola).
 *      Es mas eficiente en servidores de alto trafico, pero añade complejidad
 *      innecesaria para este proyecto academico.
 *   3. Usar flock() para bloquear el archivo directamente → mas lento que
 *      un mutex en memoria. Descartado.
 *
 * Se eligio mutex porque es lo mas simple y directo para el caso de uso.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include "logger.h"

/* Mutex estatico: se crea una sola vez y dura todo el programa.
 * PTHREAD_MUTEX_INITIALIZER es la forma de inicializar sin llamar
 * pthread_mutex_init() explicitamente. */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Puntero al archivo de log. NULL si no se abrio ningun archivo. */
static FILE *log_fp = NULL;


/* Construye un string con la fecha y hora actual en formato legible.
 * Se usa gmtime para UTC, que es el estandar en logs de servidores. */
static void get_timestamp(char *buf, int size)
{
    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", t);
}


int logger_init(const char *log_path)
{
    if (log_path != NULL) {
        /* "a" = append: no sobreescribe si el archivo ya existe.
         * Util para que el log persista entre reinicios del servidor. */
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

    /* Bloquear: solo un hilo puede entrar aqui a la vez */
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

    /* Los errores van a stderr para no mezclarse con salida normal */
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

    /* Formato inspirado en el Combined Log Format de Apache/Nginx,
     * que es el estandar de facto en servidores web */
    printf("[%s] %s \"%s %s\" %d\n", ts, client_ip, method, uri, status_code);
    fflush(stdout);

    if (log_fp != NULL) {
        fprintf(log_fp, "[%s] %s \"%s %s\" %d\n",
                ts, client_ip, method, uri, status_code);
        fflush(log_fp);
    }

    pthread_mutex_unlock(&log_mutex);
}
