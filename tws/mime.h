#ifndef MIME_H
#define MIME_H

/*
 * mime.h - Mapeo de extensiones de archivo a MIME types (HU-E3-10)
 *
 * Por que necesitamos MIME types?
 * HTTP incluye un header "Content-Type" en la respuesta para decirle al
 * browser QUE tipo de dato esta recibiendo. Sin el, el browser no sabe si
 * mostrar el archivo como HTML, renderizar la imagen, aplicar el CSS, etc.
 * Un browser moderno intentara adivinar el tipo ("sniffing"), pero no es
 * confiable y es mala practica no incluirlo.
 */

/* Retorna el string del MIME type segun la extension del archivo.
 * filename puede ser la ruta completa, solo busca la ultima extension.
 * Si la extension no esta en la tabla → "application/octet-stream"
 * (que le dice al browser "descarga esto como binario"). */
const char *get_mime_type(const char *filename);

#endif
