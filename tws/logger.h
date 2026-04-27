#ifndef LOGGER_H
#define LOGGER_H

/*
 * logger.h - Sistema de registro del TWS
 *
 * Por que necesitamos un logger separado?
 * Porque el servidor atiende peticiones con multiples hilos al mismo tiempo.
 * Si dos hilos llaman printf() simultaneamente, los textos se mezclan en la
 * salida. El mutex garantiza que solo un hilo escribe a la vez.
 *
 * Ademas el enunciado pide registrar en consola Y en archivo de forma
 * simultanea, entonces centralizar eso aqui evita repetir codigo.
 */

/* Inicializa el logger. log_path = ruta al archivo de log.
 * Retorna 0 si todo bien, -1 si no pudo abrir el archivo. */
int logger_init(const char *log_path);

/* Cierra el archivo de log (llamar al terminar el servidor). */
void logger_close(void);

/* Registra un mensaje informativo. Acepta formato printf. */
void log_info(const char *fmt, ...);

/* Registra un error. Acepta formato printf. */
void log_error(const char *fmt, ...);

/*
 * Registra una peticion HTTP completada.
 * Formato: [timestamp] IP_CLIENTE "METODO URI" CODIGO_RESPUESTA
 * Ejemplo: [2026-04-22 10:30:01] 192.168.1.5 "GET /index.html" 200
 */
void log_request(const char *client_ip, const char *method,
                 const char *uri, int status_code);

#endif
