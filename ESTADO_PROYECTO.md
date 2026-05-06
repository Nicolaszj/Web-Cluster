# Análisis de Estado del Proyecto: PIBL-WS

Este documento compara los requerimientos del PDF `ProyectoN1-PILB-WS-v1.0.pdf` con la implementación actual en el repositorio.

## 1. Servidor Web (TWS - Telematics Web Server)

| Requerimiento | Estado | Notas |
| :--- | :--- | :--- |
| Lenguaje C o Rust | ✅ Completado | Implementado en C. |
| API Sockets | ✅ Completado | Uso nativo de `sys/socket.h`. |
| Concurrencia (Hilos) | ✅ Completado | Implementado con `pthread` (Thread-per-connection). |
| Soporte HTTP/1.1 | ⚠️ Parcial | Requiere validación de `HEAD` y `POST`. `GET` funcional. |
| Códigos de error (200, 400, 404) | ✅ Completado | Implementado en `http_response.c`. |
| Sistema de Logging | ✅ Completado | Implementado en `logger.c`. |
| DocumentRoot configurable | ✅ Completado | Argumento por línea de comandos. |

## 2. Proxy Inverso + Balanceador de Carga (PIBL)

| Requerimiento | Estado | Notas |
| :--- | :--- | :--- |
| Escuchar en puerto 80/8080 | ⚠️ Pendiente | Falta el archivo principal del Proxy. |
| Balanceo Round Robin | ⚠️ Pendiente | La lógica de selección de backend no está implementada. |
| Reenvío de peticiones | ⚠️ Pendiente | Falta la lógica de socket cliente hacia los backends. |
| Logging (Stdout + Archivo) | ✅ Estructura lista | El módulo `logger.c` puede ser reutilizado. |
| Configuración parametrizada | ⚠️ Parcial | Existe `config.c` pero no está integrado con un servidor activo. |

## 3. Sistema de Caché (PIBL)

| Requerimiento | Estado | Notas |
| :--- | :--- | :--- |
| Almacenamiento en disco | ✅ Completado | Implementado en `cache.c`. |
| Persistencia tras fallos | ✅ Completado | Los archivos `.meta` guardan el estado en disco. |
| Mecanismo de TTL | ✅ Completado | Implementado con verificación de timestamp en cada lookup. |
| TTL parametrizable | ✅ Completado | Se pasa como argumento en `cache_init`. |
| Control de concurrencia | ✅ Completado | Uso de `pthread_cond_wait` para evitar doble escritura. |

## 4. Pendientes Críticos para la Entrega

1. **Motor del Proxy (`proxy_main.c`):** Necesitamos crear el ejecutable que reciba peticiones y las reparta entre los TWS.
2. **Integración de Configuración:** Cargar la lista de servidores backend desde el archivo `.conf`.
3. **Casos de Prueba:** Preparar el entorno para servir los 4 casos del PDF (imágenes, archivos de 1MB, etc.).
4. **Despliegue en AWS:** Configurar las instancias EC2 (3 para TWS, 1 para PIBL).

---
**Fecha de entrega:** 6 de Mayo de 2026 (¡Es hoy!)
**Prioridad inmediata:** Implementar el bucle principal del Proxy para unir las piezas.
