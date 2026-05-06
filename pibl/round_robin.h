/*
 * round_robin.h - Seleccion de backends
 */

#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "config.h"

/*
 * Retorna el siguiente backend a utilizar siguiendo el algoritmo Round Robin.
 * Es thread-safe (usa mutex).
 */
Backend *obtener_siguiente_backend(ConfigProxy *config);

#endif
