/*
 * mime.c - Tabla de MIME types para el TWS
 *
 * Implementacion: array estatico de structs (par extension → tipo).
 *
 * Alternativas:
 *   1. Leer /etc/mime.types de Linux → dependencia del sistema, mas complejo.
 *   2. Usar una libreria como libmagic → prohibido por el enunciado.
 *   3. Un hash map para busqueda O(1) → sobre-ingenieria para ~15 entradas.
 *      Busqueda lineal en un array de 15 elementos es practicamente igual
 *      de rapida en hardware moderno (todo cabe en cache L1).
 *
 * Se eligio el array estatico porque es simple, sin dependencias y suficiente.
 */

#include <string.h>
#include <strings.h>
#include "mime.h"

typedef struct {
    const char *extension;
    const char *tipo;
} EntradaMime;

/*
 * Tabla de MIME types. El ultimo elemento tiene NULL en ambos campos,
 * eso es el "centinela" que indica el fin del array al recorrerlo.
 * No se necesita saber el tamanio en tiempo de compilacion.
 */
static const EntradaMime tabla_mime[] = {
    { ".html",  "text/html; charset=utf-8"      },
    { ".htm",   "text/html; charset=utf-8"      },
    { ".css",   "text/css"                      },
    { ".js",    "application/javascript"        },
    { ".json",  "application/json"              },
    { ".png",   "image/png"                     },
    { ".jpg",   "image/jpeg"                    },
    { ".jpeg",  "image/jpeg"                    },
    { ".gif",   "image/gif"                     },
    { ".ico",   "image/x-icon"                  },
    { ".svg",   "image/svg+xml"                 },
    { ".txt",   "text/plain; charset=utf-8"     },
    { ".pdf",   "application/pdf"               },
    { ".zip",   "application/zip"               },
    { ".mp4",   "video/mp4"                     },
    { NULL,     NULL                            }
};


const char *get_mime_type(const char *filename)
{
    if (filename == NULL)
        return "application/octet-stream";

    /* strrchr encuentra la ultima ocurrencia del caracter '.' en el string.
     * Necesitamos la ULTIMA para manejar nombres como "archivo.backup.html",
     * cuya extension real es ".html", no ".backup". */
    const char *ext = strrchr(filename, '.');
    if (ext == NULL)
        return "application/octet-stream";

    /* Recorrer la tabla hasta el centinela NULL */
    for (int i = 0; tabla_mime[i].extension != NULL; i++) {
        /* strcasecmp: comparacion sin distinguir mayusculas/minusculas.
         * ".HTML" y ".html" deben dar el mismo resultado. */
        if (strcasecmp(ext, tabla_mime[i].extension) == 0) {
            return tabla_mime[i].tipo;
        }
    }

    /* Extension desconocida: decirle al browser que lo descargue como binario */
    return "application/octet-stream";
}
