/*
 * logger.h - Interfaz del sistema de registro para el Proxy
 */

#ifndef LOGGER_H
#define LOGGER_H

int  logger_init(const char *log_path);
void logger_close(void);

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_request(const char *client_ip, const char *method, const char *uri, int status_code);

#endif
