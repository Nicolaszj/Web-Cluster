/*
 * round_robin.c - Implementacion de balanceo Round Robin
 *
 * El algoritmo Round Robin es el mas simple de balanceo:
 *   Peticion 1 -> Backend A
 *   Peticion 2 -> Backend B
 *   Peticion 3 -> Backend A (si solo hay 2)
 *
 * Para que funcione en un entorno multihilo (donde varios hilos pueden
 * pedir el "siguiente" al mismo tiempo), necesitamos un mutex.
 */

#include <pthread.h>
#include "round_robin.h"

static int g_indice_actual = 0;
static pthread_mutex_t g_mutex_rr = PTHREAD_MUTEX_INITIALIZER;

Backend *obtener_siguiente_backend(ConfigProxy *config)
{
    if (config == NULL || config->num_backends == 0)
        return NULL;

    pthread_mutex_lock(&g_mutex_rr);
    
    int indice = g_indice_actual % config->num_backends;
    g_indice_actual++;
    
    pthread_mutex_unlock(&g_mutex_rr);

    return &config->backends[indice];
}
