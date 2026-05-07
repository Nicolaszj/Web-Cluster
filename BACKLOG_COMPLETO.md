# Backlog Completo — Proyecto PIBL-WS
## Proxy Inverso + Balanceador de Carga + Web Server

**Repositorio:** Nicolaszj/Web-Cluster  
**Entrega:** Mayo 6, 2026  
**Total HU:** 37

### Roles
| Rol | Responsable | Módulo | Rama |
|-----|-------------|--------|------|
| Rol 1 | Nicolaszj | TWS — Telematics Web Server | `feature/parser-http` |
| Rol 2 | Elpaipsz | PIBL — Proxy + Balanceador | `feature/proxy-core` |
| Rol 3 | NavarroAbraham | Infra + Caché + Config + Docs | `feature/aws-cache` |

### Estado de HUs
| Símbolo | Significado |
|---------|-------------|
| ✅ | Completada — código implementado |
| 🔄 | En progreso |
| ⬜ | Pendiente |

---

## Cómo usar este archivo en GitHub

Cada HU está formateada para ser copiada directamente como un **Issue de GitHub**:
- El encabezado `###` es el **título del Issue**
- Todo lo demás es el **cuerpo del Issue** en Markdown
- Los `- [ ]` se convierten en checkboxes interactivos en GitHub

---

---

# ÉPICA E0 — Metodología y Gestión del Proyecto

---

### [HU-E0-01] Estrategia de ramas por módulo y reglas de integración ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham  
**Rama:** `main` (docs)  
**Prioridad:** `Crítica`  
**Etiquetas:** `metodología` `git` `calidad`  
**Archivos:** `CONTRIBUTING.md`

---

**Como** equipo de desarrollo,  
**quiero** definir y documentar una estrategia de ramas por módulo (`feature/parser-http`, `feature/proxy-core`, `feature/aws-cache`) y reglas de merge a `main`,  
**para** asegurar orden, trazabilidad y evitar integración de código que no compila o no pasa pruebas.

#### Criterios de Aceptación
- [x] Existe un documento `CONTRIBUTING.md` en la raíz del repositorio con:
  - [x] Listado de ramas: `feature/parser-http` (Rol 1), `feature/proxy-core` (Rol 2), `feature/aws-cache` (Rol 3)
  - [x] Reglas de merge: solo a `main` cuando el código compile y pase pruebas
  - [x] Prohibición explícita de compartir archivos `.c`/`.rs` por canales externos (WhatsApp, email, etc.)
  - [x] Checklist mínimo de PR definido
- [ ] (Recomendado) Protección de rama `main` habilitada en GitHub

#### Tareas
- [x] Crear `CONTRIBUTING.md` en la raíz del repositorio
- [x] Definir y documentar el checklist mínimo para PRs
- [x] Comunicar la política al equipo

#### Definición de Hecho
Documento `CONTRIBUTING.md` publicado en `main` y todo el equipo lo conoce y aplica.

---

### [HU-E0-02] Tablero Kanban con tarjetas por HU

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main` (docs)  
**Prioridad:** `Alta`  
**Etiquetas:** `metodología` `kanban` `gestión`

---

**Como** equipo de desarrollo,  
**quiero** un tablero Kanban con columnas (Backlog / ToDo / In Progress / In Review / Done) y tarjetas vinculadas a cada HU,  
**para** evidenciar gestión profesional del proyecto durante la sustentación.

#### Criterios de Aceptación
- [ ] Tablero creado en GitHub Projects y visible para el equipo
- [ ] Cada HU del backlog está vinculada como tarjeta en el tablero
- [ ] Cada tarjeta tiene un responsable asignado según los roles definidos
- [ ] Las columnas reflejan el flujo: Backlog → ToDo → In Progress → In Review → Done

#### Tareas
- [ ] Crear proyecto en GitHub Projects (vista Kanban)
- [ ] Crear un Issue por cada HU del backlog
- [ ] Asignar cada Issue al responsable correspondiente
- [ ] Agregar labels de épica y prioridad a cada Issue

#### Definición de Hecho
Tablero activo, con todas las HUs como tarjetas, responsables asignados y siendo actualizado durante el desarrollo.

---

### [HU-E0-03] Restricción de implementación desde cero (sin librerías externas) ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham  
**Rama:** `main` (docs)  
**Prioridad:** `Crítica`  
**Etiquetas:** `metodología` `calidad` `restricciones`  
**Archivos:** `CONTRIBUTING.md` (sección 2 + checklist PR)

---

**Como** equipo de desarrollo,  
**quiero** documentar y hacer cumplir la restricción de que todo el código debe ser implementado desde cero usando únicamente C (o Rust), la API POSIX de sockets y las llamadas al sistema operativo,  
**para** cumplir con la exigencia académica de no usar librerías externas, frameworks ni código generado por IA.

#### Criterios de Aceptación
- [x] Existe una sección "Restricciones de implementación" en `CONTRIBUTING.md` que prohíbe:
  - [x] Librerías HTTP externas (libcurl, mongoose, etc.)
  - [x] Parsers externos de cualquier tipo
  - [x] Frameworks web de cualquier índole
  - [x] Código generado o copiado de herramientas de IA
- [x] El código del proyecto usa únicamente: `stdlib`, `string.h`, `stdio.h`, `pthread.h`, `sys/socket.h` y syscalls POSIX estándar
- [x] El checklist de PR incluye el ítem: "¿El código usa solo stdlib + POSIX? (sin librerías externas)"
- [x] Cualquier referencia externa está citada como RFC/documentación, nunca como código

#### Tareas
- [x] Agregar sección "Restricciones de implementación" en `CONTRIBUTING.md`
- [x] Actualizar checklist de PR con verificación de esta restricción
- [x] Comunicar al equipo en la primera reunión de trabajo

#### Definición de Hecho
Restricción documentada, conocida por todos los miembros y verificada en cada PR.

---
---

# ÉPICA E1 — Documentación Base y Requisitos

---

### [HU-E1-01] Crear README.md con estructura oficial del curso

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main`  
**Prioridad:** `Crítica`  
**Etiquetas:** `documentación`

---

**Como** responsable de documentación,  
**quiero** crear el `README.md` con las secciones exactas exigidas por el curso: Introducción, Desarrollo (Arquitectura, Decisiones de Diseño, Guía de Despliegue, Casos de Prueba), Conclusiones y Referencias,  
**para** asegurar que el documento final cumple el formato oficial de entrega.

#### Criterios de Aceptación
- [ ] `README.md` existe en la raíz del repositorio
- [ ] Contiene las secciones: Introducción, Desarrollo, Conclusiones, Referencias
- [ ] La sección Desarrollo incluye subsecciones: Arquitectura, Decisiones de Diseño, Guía de Despliegue, Casos de Prueba
- [ ] Cada sección tiene al menos un placeholder o TODO que indica qué contenido va ahí
- [ ] La sección Referencias incluye: RFC 2616 o RFC 9110 (HTTP/1.1), POSIX.1-2017, documentación AWS EC2 y material del curso

#### Tareas
- [ ] Crear esqueleto del `README.md` con todas las secciones
- [ ] Agregar placeholders en cada sección
- [ ] Completar la sección Referencias con: RFC 2616/9110, POSIX.1-2017, AWS EC2, material del curso
- [ ] Insertar diagramas UML cuando estén listos (HU-E2-01, E2-02, E2-03)

#### Definición de Hecho
`README.md` con estructura completa publicado en `main`. Listo para ser completado conforme avanza el proyecto.

---

### [HU-E1-02] Especificar Requisitos Funcionales y No Funcionales

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main`  
**Prioridad:** `Alta`  
**Etiquetas:** `documentación` `requisitos`

---

**Como** equipo de desarrollo,  
**quiero** documentar los RF (funcionales) y RNF (no funcionales) del sistema,  
**para** demostrar comprensión de los requisitos y justificar las decisiones de diseño tomadas.

#### Criterios de Aceptación
- [ ] RF cubren: TWS (GET/HEAD/POST, 200/400/404, DocumentRoot), Proxy (Round Robin, caché+TTL, configuración), logs, despliegue en AWS
- [ ] RNF cubren: concurrencia (mínimo 10 clientes simultáneos), robustez ante caídas de backend, persistencia del caché, estabilidad con archivos ≥ 1MB, observabilidad mediante logs, reproducibilidad del despliegue
- [ ] Los RF y RNF están listados en una sección del `README.md`

#### Tareas
- [ ] Redactar lista de RF (mínimo 10 requisitos)
- [ ] Redactar lista de RNF (mínimo 5 requisitos)
- [ ] Agregar la sección al `README.md`

#### Definición de Hecho
Sección RF/RNF completa y publicada en `README.md`.

---
---

# ÉPICA E2 — Diagramas UML Obligatorios

---

### [HU-E2-01] Diagrama UML de Despliegue (Deployment Diagram)

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main`  
**Prioridad:** `Alta`  
**Etiquetas:** `uml` `arquitectura` `diagramas`

---

**Como** responsable de documentación,  
**quiero** un diagrama de despliegue UML que muestre los nodos físicos: Cliente, AWS Cloud, EC2-Proxy (PIBL) y EC2-WS1/WS2/WS3, con sus puertos y protocolos de comunicación,  
**para** cumplir el requisito principal de documentación de arquitectura del curso.

#### Criterios de Aceptación
- [ ] El diagrama muestra los nodos: Cliente, AWS Cloud, EC2-Proxy, EC2-WS1, EC2-WS2, EC2-WS3
- [ ] Incluye los enlaces con protocolo HTTP/TCP
- [ ] Muestra los puertos: cliente → proxy (80/8080), proxy → backends (puertos configurados)
- [ ] Los archivos fuente editables y el export PNG están guardados en `diagramas/`
- [ ] El diagrama está insertado y explicado en el `README.md`

#### Tareas
- [ ] Crear el diagrama con draw.io, PlantUML o similar
- [ ] Exportar a PNG
- [ ] Guardar fuente editable en `diagramas/`
- [ ] Insertar en `README.md` con explicación

#### Definición de Hecho
Diagrama versionado en `diagramas/` y referenciado desde `README.md`.

---

### [HU-E2-02] Diagrama UML de Secuencia (flujo con caché HIT/MISS/TTL)

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main`  
**Prioridad:** `Alta`  
**Etiquetas:** `uml` `secuencia` `caché`

---

**Como** responsable de documentación,  
**quiero** un diagrama de secuencia UML que muestre el flujo completo de una petición (cliente → proxy → caché → Round Robin → backend → proxy → cliente),  
**para** explicar el comportamiento real del sistema incluyendo los escenarios de caché.

#### Criterios de Aceptación
- [ ] El diagrama incluye las líneas de vida: Cliente, Proxy PIBL, Caché, Backend TWS
- [ ] Incluye el fragmento `alt` para el caso CACHE HIT (no va al backend)
- [ ] Incluye el fragmento `alt` para CACHE MISS (va al backend y luego almacena)
- [ ] Incluye el fragmento para expiración de TTL (MISS forzado)
- [ ] Guardado en `diagramas/` y explicado en `README.md`

#### Tareas
- [ ] Crear el diagrama con PlantUML o draw.io
- [ ] Incluir los tres escenarios (HIT, MISS, TTL expirado)
- [ ] Exportar PNG y guardar fuente
- [ ] Insertar en `README.md`

#### Definición de Hecho
Diagrama publicado y explicado en `README.md`.

---

### [HU-E2-03] Diagrama UML de Componentes (estructura interna del sistema)

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Rama:** `main`  
**Prioridad:** `Media`  
**Etiquetas:** `uml` `componentes` `arquitectura`

---

**Como** equipo de desarrollo,  
**quiero** un diagrama de componentes UML que muestre los módulos internos del sistema (Sockets, Parser HTTP, Round Robin, Caché I/O, Logger, Config),  
**para** evidenciar diseño modular y las dependencias entre los módulos durante la sustentación.

#### Criterios de Aceptación
- [ ] El diagrama muestra los módulos del TWS: `server.c`, `http_parser`, `http_response`, `mime`, `logger`
- [ ] El diagrama muestra los módulos del PIBL: socket server/client, config, round robin, caché, logger
- [ ] Las interfaces y dependencias entre módulos están representadas
- [ ] Guardado en `diagramas/` (fuente + PNG) y referenciado en `README.md`

#### Tareas
- [ ] Crear diagrama para el TWS
- [ ] Crear diagrama para el PIBL
- [ ] Exportar PNG y guardar fuentes
- [ ] Insertar en `README.md`

#### Definición de Hecho
Diagramas publicados y referenciados en `README.md`.

---
---

# ÉPICA E3 — TWS (Telematics Web Server) — Rol 1: @Nicolaszj

---

### [HU-E3-01] Esqueleto del TWS con CLI exacto de ejecución ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Etiquetas:** `tws` `setup` `cli`  
**Archivos:** `tws/server.c`

---

**Como** desarrollador del TWS,  
**quiero** crear el esqueleto del servidor que compile y reciba exactamente `./server <HTTP_PORT> <Log_File> <DocumentRootFolder>`,  
**para** cumplir el contrato de ejecución exigido por el enunciado y habilitar el desarrollo incremental.

#### Criterios de Aceptación
- [x] Compila en Linux (local y EC2) sin errores ni warnings
- [x] Valida los 3 argumentos y muestra mensaje de uso si faltan o son incorrectos
- [x] Arranca correctamente con puerto, abre el archivo de log y guarda el DocumentRoot
- [x] Muestra mensaje de inicio con puerto, ruta de log y DocumentRoot en consola

#### Tareas
- [x] Crear `server.c` con función `main()` y validación de argumentos
- [x] Integrar inicialización del logger con la ruta recibida por CLI
- [x] Guardar DocumentRoot en variable global accesible por `http_response.c`
- [x] Verificar que el puerto esté en el rango válido (1–65535)

#### Definición de Hecho
Binario `server` ejecutable. `./server 8080 server.log ./www` arranca sin errores.

---

### [HU-E3-02] Socket servidor TCP: bind/listen/accept robusto ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-01  
**Etiquetas:** `tws` `sockets` `tcp`  
**Archivos:** `tws/server.c`

---

**Como** desarrollador del TWS,  
**quiero** implementar un socket TCP servidor que acepte conexiones de clientes (browser, curl, telnet, Postman),  
**para** poder recibir peticiones HTTP desde cualquier cliente compatible.

#### Criterios de Aceptación
- [x] `SO_REUSEADDR` configurado para evitar "Address already in use" al reiniciar
- [x] Maneja errores de `bind()` (puerto ocupado, permisos) y termina con mensaje claro
- [x] Maneja errores de `listen()` con mensaje descriptivo
- [x] El bucle de `accept()` continúa operando ante errores individuales (no termina el servidor)
- [x] `INADDR_ANY` para escuchar en todas las interfaces de red

#### Tareas
- [x] Crear socket con `socket(AF_INET, SOCK_STREAM, 0)`
- [x] Configurar `SO_REUSEADDR` con `setsockopt()`
- [x] Implementar `bind()` con manejo de errores
- [x] Implementar `listen()` con backlog de 128
- [x] Bucle principal con `accept()` y manejo de errores sin terminar el servidor

#### Definición de Hecho
El servidor acepta conexiones TCP en el puerto configurado. Verificado con `telnet localhost 8080`.

---

### [HU-E3-03] Parser HTTP/1.1: métodos GET, HEAD y POST ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-02  
**Etiquetas:** `tws` `http` `parser`  
**Archivos:** `tws/http_parser.c`, `tws/http_parser.h`

---

**Como** desarrollador del TWS,  
**quiero** parsear correctamente peticiones HTTP/1.1 con los métodos GET, HEAD y POST, extrayendo la línea de petición, todos los headers y el body,  
**para** interpretar las solicitudes de los clientes de forma robusta y conforme al RFC 2616.

#### Criterios de Aceptación
- [x] Reconoce y diferencia los 3 métodos: GET, HEAD, POST
- [x] Extrae la URI correctamente de la línea de petición
- [x] Valida que la versión del protocolo comience con `HTTP/`
- [x] Para POST: usa el valor de `Content-Length` para leer exactamente el body completo
- [x] POST sin `Content-Length` → la petición se marca como inválida (respuesta 400)
- [x] Petición malformada (sin `\r\n`, sin los 3 campos) → marcada como inválida
- [x] Bucle de lectura del body para manejar `recv()` parciales de TCP
- [x] Límite de 10 MB para el body (protección contra DoS)

#### Tareas
- [x] Implementar `leer_headers_socket()` — lectura byte a byte hasta `\r\n\r\n`
- [x] Implementar `parsear_linea_peticion()` — extraer método, URI y versión
- [x] Implementar `parsear_headers()` — extraer pares nombre:valor
- [x] Implementar lectura de body para POST con bucle acumulador
- [x] Implementar `buscar_header()` — búsqueda case-insensitive
- [x] Implementar `liberar_peticion()` — liberar el body si fue asignado con `malloc()`

#### Definición de Hecho
Parser estable verificado manualmente con `curl`, telnet y Postman para los 3 métodos.

---

### [HU-E3-04] Servir recursos desde DocumentRootFolder (texto y binarios) ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-03  
**Etiquetas:** `tws` `file-io` `docroot`  
**Archivos:** `tws/http_response.c`

---

**Como** desarrollador del TWS,  
**quiero** resolver rutas relativas de la URI dentro del DocumentRootFolder y servir los archivos correspondientes (HTML, imágenes, CSS, JS, binarios),  
**para** cumplir la función principal del servidor web.

#### Criterios de Aceptación
- [x] URI `/` sirve automáticamente `index.html` del DocumentRoot
- [x] Rechaza path traversal con `..` (responde 400 Bad Request)
- [x] Sirve correctamente archivos binarios (imágenes, `.bin`)
- [x] Archivos de contenido ≥ 1 MB se sirven completos, sin truncar (casos 3 y 4)
- [x] Envío en chunks de 8 KB para no cargar todo el archivo en memoria
- [x] Bucle `send()` para manejar envíos parciales de TCP

#### Tareas
- [x] Implementar `uri_es_segura()` — rechazar `..` y `//`
- [x] Implementar `resolver_ruta()` — concatenar DocumentRoot + URI
- [x] Usar `stat()` para verificar existencia y tipo del archivo
- [x] Usar `open()` + `read()` en bucle de chunks de 8 KB
- [x] Implementar `enviar_todo()` — bucle interno para envíos parciales de TCP

#### Definición de Hecho
Los 4 casos de prueba del enunciado pueden ser servidos correctamente.

---

### [HU-E3-05] Respuestas HTTP correctas: 200, 400, 404 y HEAD sin body ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-03, HU-E3-04  
**Etiquetas:** `tws` `http` `errores`  
**Archivos:** `tws/http_response.c`

---

**Como** desarrollador del TWS,  
**quiero** responder con los códigos de estado HTTP correctos (200/400/404) según el resultado de cada petición,  
**para** cumplir el requisito de manejo robusto de errores del enunciado.

#### Criterios de Aceptación
- [x] 200 OK cuando el recurso existe y se sirve correctamente
- [x] 404 Not Found cuando el recurso solicitado no existe en el DocumentRoot
- [x] 400 Bad Request cuando la petición está malformada, la URI es insegura, o POST no tiene `Content-Length`
- [x] HEAD: headers idénticos al GET equivalente (mismo `Content-Length`) pero body vacío
- [x] Todas las respuestas de error incluyen un body HTML mínimo con el código y mensaje
- [x] Todas las respuestas incluyen los headers: `Date`, `Server`, `Content-Type`, `Content-Length`, `Connection: close`

#### Tareas
- [x] Implementar `enviar_error()` — respuesta de error genérica con body HTML
- [x] Implementar lógica de 200/404 en `manejar_get_head()`
- [x] Manejar HEAD con parámetro `send_body=0` (no envía el cuerpo)
- [x] Devolver 400 en `procesar_peticion()` cuando `peticion->valida == 0`
- [x] Verificar con `curl -v` que todos los casos responden correctamente

#### Definición de Hecho
`curl -v http://IP:8080/recurso` muestra el código de respuesta correcto para todos los casos.

---

### [HU-E3-06] Logger del TWS: consola + archivo con request y response ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Alta`  
**Dependencias:** HU-E3-01  
**Etiquetas:** `tws` `logging`  
**Archivos:** `tws/logger.c`, `tws/logger.h`

---

**Como** desarrollador del TWS,  
**quiero** registrar cada petición y su respuesta simultáneamente en la consola y en el archivo de log configurado,  
**para** cumplir la exigencia de logger del enunciado y facilitar la depuración durante las pruebas.

#### Criterios de Aceptación
- [x] Cada entrada de log incluye: timestamp, IP del cliente, método HTTP, URI y código de respuesta
- [x] Los errores (400/404) también quedan registrados
- [x] El formato es consistente en todas las entradas: `[YYYY-MM-DD HH:MM:SS] IP "METODO URI" CODIGO`
- [x] El logger es thread-safe: usar mutex para evitar mezcla de logs con concurrencia
- [x] Los errores van a `stderr` y la info a `stdout`
- [x] `fflush()` después de cada escritura para garantizar que no se pierdan logs ante fallos

#### Tareas
- [x] Implementar `logger_init()` — abrir archivo en modo append
- [x] Implementar `log_info()` y `log_error()` con `va_list` para formato printf
- [x] Implementar `log_request()` — registro estándar de peticiones HTTP
- [x] Implementar `logger_close()` — cerrar el archivo al terminar
- [x] Mutex `pthread_mutex_t` para thread-safety

#### Definición de Hecho
Log visible en consola y en archivo durante pruebas. Entradas consistentes sin mezcla al tener 10+ clientes simultáneos.

---

### [HU-E3-07] Concurrencia por hilos en el TWS (thread-per-connection) ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-02, HU-E3-06  
**Etiquetas:** `tws` `concurrencia` `threads`  
**Archivos:** `tws/server.c`

---

**Como** desarrollador del TWS,  
**quiero** atender múltiples clientes de forma concurrente usando el modelo thread-per-connection con POSIX threads (`pthread`),  
**para** cumplir el requisito de concurrencia thread-based del enunciado.

#### Criterios de Aceptación
- [x] Por cada conexión entrante se crea un hilo con `pthread_create()`
- [x] Los datos de la conexión se pasan al hilo por heap (`malloc`) para evitar condiciones de carrera
- [x] `pthread_detach()` garantiza que los recursos del hilo se liberan automáticamente al terminar
- [x] El logger usa mutex (`pthread_mutex_t`) para ser thread-safe
- [x] El servidor maneja correctamente 10+ clientes simultáneos sin crashes
- [x] Un error en la creación de un hilo no termina el servidor (solo descarta esa conexión)

#### Tareas
- [x] Crear estructura `DatosConexion` con `sockfd` e `ip_cliente`
- [x] `malloc(sizeof(DatosConexion))` por cada conexión antes de `pthread_create()`
- [x] Función `atender_conexion()` que libera el `malloc`, procesa y cierra el socket
- [x] `pthread_detach()` inmediatamente después de `pthread_create()`
- [ ] **Pendiente:** Ejecutar prueba de estrés con 30+ clientes simultáneos y documentar evidencia

#### Definición de Hecho
Prueba de estrés con 30+ clientes concurrentes documentada en el README (sección Casos de Prueba). Sin crashes ni logs mezclados.

---

### [HU-E3-08] Paquete de pruebas del TWS con evidencia ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Alta`  
**Dependencias:** HU-E3-05, HU-E3-07  
**Etiquetas:** `tws` `testing` `evidencia`  
**Archivos:** `tws/test_tws.sh`, `README.md`

---

**Como** desarrollador del TWS,  
**quiero** ejecutar y documentar pruebas del servidor con múltiples tipos de clientes (curl, Postman, browser, telnet, wireshark),  
**para** garantizar interoperabilidad y tener evidencia concreta para la entrega y sustentación.

#### Criterios de Aceptación
- [x] Evidencia de petición GET con respuesta 200 y el recurso correcto
- [x] Evidencia de petición HEAD con headers idénticos al GET pero sin body
- [x] Evidencia de petición POST con respuesta 200 y confirmación JSON
- [x] Evidencia de respuesta 404 para un recurso que no existe
- [x] Evidencia de respuesta 400 para una petición malformada
- [x] Evidencia de servicio de un archivo de ≥ 1 MB completo (sin truncar)
- [x] Extracto del log del servidor mostrando las peticiones registradas
- [x] Toda la evidencia (comandos usados + salidas) documentada en `README.md`

#### Tareas
- [x] Ejecutar pruebas con `curl -v` para GET, HEAD, POST, 404, 400
- [x] Ejecutar prueba de descarga de archivo de 1 MB y verificar integridad con `md5sum`
- [x] Ejecutar prueba de estrés concurrente (30+ peticiones simultáneas)
- [x] Documentar comandos y evidencia en la sección "Casos de Prueba" del README (`tws/test_tws.sh`)

#### Definición de Hecho
Sección "Casos de Prueba" del README con evidencia real de todos los escenarios ejecutados en AWS.

---

### [HU-E3-09] Procesamiento de body POST y respuesta 200 con confirmación ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-03, HU-E3-05  
**Etiquetas:** `tws` `http` `post` `body`  
**Archivos:** `tws/http_parser.c`, `tws/http_response.c`

---

**Como** desarrollador del TWS,  
**quiero** que el servidor reciba el body completo de una petición POST, lo procese correctamente y responda con código 200 y un body de confirmación,  
**para** demostrar manejo robusto del método POST de extremo a extremo.

#### Criterios de Aceptación
- [x] El servidor lee el body completo usando el valor exacto de `Content-Length`
- [x] POST sin `Content-Length` → responde 400 Bad Request
- [x] Body recibido correctamente → responde 200 con `{"status":"received","mensaje":"Datos procesados OK"}`
- [x] El body recibido y el status de respuesta quedan registrados en el logger
- [ ] **Pendiente:** Validado con `curl -X POST -d "dato=valor" http://IP:PORT/endpoint`

#### Tareas
- [x] Implementar lectura completa del body basada en `Content-Length` con bucle acumulador
- [x] Implementar `manejar_post()` que genera la respuesta JSON de confirmación
- [x] Agregar el tamaño del body al log de la petición POST
- [ ] Documentar prueba con `curl` en README

#### Definición de Hecho
POST responde 200 con confirmación JSON; 400 si malformado. Evidencia en logs y en README.

---

### [HU-E3-10] Mapeo de extensiones a MIME types (Content-Type correcto) ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-04  
**Etiquetas:** `tws` `http` `mime` `content-type`  
**Archivos:** `tws/mime.c`, `tws/mime.h`

---

**Como** desarrollador del TWS,  
**quiero** implementar un módulo que mapee extensiones de archivo a su MIME type correcto,  
**para** que el header `Content-Type` de cada respuesta sea correcto y los browsers rendericen los recursos adecuadamente.

#### Criterios de Aceptación
- [x] El servidor incluye `Content-Type` en todas las respuestas 200
- [x] Cubre las extensiones de los casos de prueba: `.html`, `.css`, `.js`, `.jpg`, `.jpeg`, `.png`, `.gif`, `.ico`, `.txt`, `.pdf`
- [x] Extensión desconocida → `application/octet-stream`
- [x] La búsqueda es case-insensitive (`.HTML` == `.html`)
- [x] Implementado como array estático de structs sin librerías externas
- [ ] **Pendiente:** Validado en browser real: imágenes y HTML renderizan correctamente en los casos 1 y 2

#### Tareas
- [x] Crear `get_mime_type(const char *filename)` con `strrchr()` para extraer extensión
- [x] Implementar tabla estática de structs `{extensión, tipo}` con centinela NULL
- [x] Usar `strcasecmp()` para comparación sin distinción de mayúsculas
- [x] Integrar en `http_response.c` al construir los headers de respuesta

#### Definición de Hecho
Browser renderiza correctamente HTML e imágenes en los 4 casos de prueba. Header `Content-Type` verificable con `curl -v`.

---

### [HU-E3-11] Header Content-Length correcto en todas las respuestas HTTP ✅

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-04, HU-E3-05  
**Etiquetas:** `tws` `http` `headers` `content-length`  
**Archivos:** `tws/http_response.c`

---

**Como** desarrollador del TWS,  
**quiero** incluir el header `Content-Length` con el valor exacto en bytes en todas las respuestas HTTP,  
**para** que los clientes HTTP/1.1 (curl, browser, Postman, el proxy PIBL) sepan exactamente cuándo termina la respuesta.

#### Criterios de Aceptación
- [x] Respuestas 200 incluyen `Content-Length: N` donde N es el tamaño real del archivo en bytes
- [x] Respuestas 400 y 404 incluyen `Content-Length` del tamaño de su body de error
- [x] HEAD incluye `Content-Length` idéntico al que tendría el GET, pero sin body
- [x] Para archivos ≥ 1 MB el valor es exacto (usando `stat()`)
- [ ] **Pendiente:** Validado con `curl -v` verificando que la conexión cierra limpiamente sin timeout

#### Tareas
- [x] Usar `stat()` para obtener el tamaño exacto del archivo antes de construir headers
- [x] Incluir `Content-Length` en `snprintf()` del constructor de headers
- [x] Incluir `Content-Length` en `enviar_error()` para respuestas de error
- [ ] Probar con `curl -v` archivos de 1 MB+ que la conexión cierre correctamente

#### Definición de Hecho
`curl -v` muestra `Content-Length` correcto. Sin timeouts ni conexiones colgadas.

---
---

# ÉPICA E4 — PIBL Proxy + Balanceador — Rol 2: @Elpaipsz

---

### [HU-E4-01] Esqueleto del Proxy PIBL (módulos base + compilación) ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Etiquetas:** `proxy` `setup`

---

**Como** desarrollador del proxy PIBL,  
**quiero** crear el `main` y los módulos base (socket server/client, config, Round Robin, logger, hooks de caché),  
**para** construir el intermediario de forma escalable y modular.

#### Criterios de Aceptación
- [x] El proyecto compila en Linux (local y EC2) sin errores ni warnings
- [x] Estructura modular con archivos separados por responsabilidad
- [x] El binario arranca y muestra un mensaje de inicio

#### Tareas
- [x] Crear estructura de directorios para el PIBL (`pibl/`)
- [x] Crear `main.c` con punto de entrada
- [x] Crear módulos vacíos: `config.c`, `round_robin.c`, `logger.c`, `cache.c`
- [x] Crear `Makefile` equivalente al del TWS

#### Definición de Hecho
Proyecto compila limpio. Estructura lista para desarrollo incremental.

---

### [HU-E4-02] Archivo de configuración PIBL: puerto + lista de backends ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-01  
**Etiquetas:** `proxy` `config`

---

**Como** desarrollador del proxy PIBL,  
**quiero** leer un archivo de configuración que defina el puerto de escucha y la lista de IPs/puertos de los backends,  
**para** parametrizar el proxy sin necesidad de recompilar.

#### Criterios de Aceptación
- [ ] El proxy lee el archivo de configuración al iniciar
- [ ] La configuración define: puerto de escucha, lista de backends (IP:puerto), TTL del caché
- [ ] Errores en el archivo de configuración (formato inválido, backends vacíos) terminan el proceso con mensaje claro
- [ ] El formato del archivo está documentado en el README con un ejemplo (`config.example`)

#### Tareas
- [x] Definir formato del archivo de configuración
- [x] Implementar parser del archivo de configuración
- [x] Validar los campos obligatorios
- [x] Crear `config.example` con documentación de cada campo

#### Definición de Hecho
El proxy carga la configuración correctamente. `config.example` documentado en el README.

---

### [HU-E4-03] Socket servidor PIBL: escucha en 80/8080 + aceptación concurrente ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-02  
**Etiquetas:** `proxy` `sockets` `concurrencia`

---

**Como** desarrollador del proxy PIBL,  
**quiero** aceptar conexiones concurrentes en el puerto configurado (80 u 8080) usando thread-per-connection,  
**para** manejar múltiples peticiones simultáneas de clientes.

#### Criterios de Aceptación
- [ ] El proxy escucha en el puerto definido en el archivo de configuración
- [ ] Modelo thread-per-connection con `pthread_create()` + `pthread_detach()`
- [ ] Maneja correctamente 10+ conexiones simultáneas sin crashes
- [ ] `SO_REUSEADDR` configurado

#### Tareas
- [x] Implementar socket servidor con `socket()`, `bind()`, `listen()`, `accept()`
- [x] Bucle principal con creación de hilo por conexión
- [x] `DatosConexion` pasado por heap al hilo

#### Definición de Hecho
Proxy acepta 10+ conexiones simultáneas verificado con prueba básica.

---

### [HU-E4-04] Round Robin thread-safe (sin saltos bajo concurrencia) ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-02  
**Etiquetas:** `proxy` `round-robin` `concurrencia`

---

**Como** desarrollador del proxy PIBL,  
**quiero** distribuir las peticiones de forma secuencial entre WS1, WS2 y WS3 usando Round Robin con sincronización,  
**para** balancear la carga de forma equitativa conforme al enunciado.

#### Criterios de Aceptación
- [ ] La distribución es secuencial: petición 1 → WS1, petición 2 → WS2, petición 3 → WS3, petición 4 → WS1...
- [ ] El contador Round Robin está protegido con mutex o variable atómica para evitar condiciones de carrera
- [ ] Los logs evidencian la distribución correcta entre los 3 backends

#### Tareas
- [x] Implementar contador global con mutex `pthread_mutex_t`
- [x] Función `seleccionar_backend()` que retorna el siguiente backend según RR
- [x] Registrar el backend seleccionado en el log de cada petición

#### Definición de Hecho
Los logs muestran distribución RR 1/3/1/3/1/3... bajo concurrencia. Sin saltos ni repeticiones incorrectas.

---

### [HU-E4-05] Interceptación y reenvío de request intacta al backend ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-03  
**Etiquetas:** `proxy` `http` `forward`

---

**Como** proxy PIBL,  
**quiero** leer la petición HTTP completa del cliente y reenviarla sin modificar al backend seleccionado,  
**para** actuar como un intermediario transparente.

#### Criterios de Aceptación
- [ ] El proxy lee la petición HTTP completa del cliente (headers + body si aplica)
- [ ] La petición se reenvía byte a byte al backend sin modificaciones
- [ ] Errores en la lectura de la petición → responde 400 al cliente

#### Tareas
- [x] Leer petición completa del socket del cliente hasta `\r\n\r\n` (+ body si Content-Length presente)
- [x] Reenviar la petición al socket del backend seleccionado
- [x] Manejar errores de lectura con respuesta 400

#### Definición de Hecho
`curl http://IP_PROXY:8080/recurso` retorna el mismo recurso que `curl http://IP_BACKEND:PORT/recurso`.

---

### [HU-E4-06] Conexión socket cliente a backend + fallback si backend cae ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Alta`  
**Dependencias:** HU-E4-04, HU-E4-05  
**Etiquetas:** `proxy` `resiliencia` `fallback`

---

**Como** proxy PIBL,  
**quiero** conectarme al backend seleccionado y, si la conexión falla, intentar con el siguiente backend de la lista,  
**para** proveer tolerancia básica a fallos sin interrumpir el servicio.

#### Criterios de Aceptación
- [ ] Si el backend seleccionado rechaza la conexión → intenta con el siguiente
- [ ] Reintenta con todos los backends antes de responder error al cliente
- [ ] Si todos los backends fallan → responde 502 Bad Gateway al cliente
- [ ] Cada intento fallido queda registrado en el log

#### Tareas
- [x] Bucle de intento de conexión que recorre la lista de backends
- [x] Manejo de `connect()` fallido con registro en log
- [x] Respuesta 502 si todos los backends están caídos

#### Definición de Hecho
Apagando un EC2 backend, el proxy redirige automáticamente al siguiente. Evidencia en el log.

---

### [HU-E4-07] Relay de respuesta backend → cliente (sin modificar) ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-06  
**Etiquetas:** `proxy` `relay`

---

**Como** proxy PIBL,  
**quiero** retransmitir la respuesta del backend intacta al cliente original,  
**para** cumplir el comportamiento de proxy inverso transparente.

#### Criterios de Aceptación
- [ ] La respuesta se transmite en chunks de al menos 8 KB
- [ ] Soporta respuestas con archivos binarios y archivos ≥ 1 MB
- [ ] No modifica los headers ni el body de la respuesta
- [ ] Cierra ambos sockets (cliente y backend) al finalizar

#### Tareas
- [x] Bucle de lectura del socket del backend y escritura al socket del cliente
- [x] Usar buffer de al menos 8 KB para el relay
- [x] Cerrar ambos sockets al final

#### Definición de Hecho
Prueba E2E: cliente → proxy → backend → proxy → cliente funcionando para los 4 casos de prueba.

---

### [HU-E4-08] Logger del Proxy: consola + archivo, request + response ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Alta`  
**Dependencias:** HU-E4-03  
**Etiquetas:** `proxy` `logging`

---

**Como** proxy PIBL,  
**quiero** registrar cada petición recibida y la respuesta enviada, incluyendo el backend elegido,  
**para** tener auditoría completa del tráfico durante pruebas y sustentación.

#### Criterios de Aceptación
- [ ] Cada entrada incluye: timestamp, IP cliente, método, URI, backend seleccionado, código de respuesta
- [ ] El logger es thread-safe (mutex)
- [ ] Registra simultáneamente en consola y archivo
- [ ] `fflush()` después de cada escritura

#### Tareas
- [x] Reusar o adaptar el módulo logger del TWS
- [x] Agregar campo de backend elegido al formato de log
- [x] Verificar thread-safety bajo concurrencia

#### Definición de Hecho
Log consistente del proxy durante pruebas de estrés. Sin entradas mezcladas.

---

### [HU-E4-09] Timeout en conexión socket cliente → backend ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-06  
**Etiquetas:** `proxy` `sockets` `resiliencia` `timeout`

---

**Como** desarrollador del proxy PIBL,  
**quiero** configurar un timeout en las operaciones de conexión y lectura del socket hacia el backend,  
**para** que el proxy nunca se bloquee indefinidamente si un backend EC2 está caído o no responde.

#### Criterios de Aceptación
- [ ] La conexión al backend tiene un timeout configurable (sugerido: 5 segundos)
- [ ] Si el timeout se cumple: cierra el socket, registra en log y ejecuta fallback al siguiente backend
- [ ] Si todos los backends están en timeout → responde 502 al cliente con mensaje claro
- [ ] Implementado con `setsockopt(SO_RCVTIMEO / SO_SNDTIMEO)` sin librerías externas
- [ ] El timeout no afecta conexiones activas en transferencia normal
- [ ] Validado apagando un EC2 backend durante una prueba

#### Tareas
- [x] Implementar `setsockopt()` con `SO_RCVTIMEO` y `SO_SNDTIMEO` al crear el socket cliente
- [x] Manejar `errno == EAGAIN / ETIMEDOUT` en `recv()` / `send()`
- [x] Integrar con la lógica de fallback de HU-E4-06
- [x] Registrar eventos de timeout en el log
- [ ] Documentar prueba de backend caído en README

#### Definición de Hecho
Con un backend apagado, el proxy redirige al siguiente en ≤ 5 segundos. Log registra el timeout. Evidencia documentada.

---

### [HU-E4-10] Prueba de estrés del Proxy: evidencia de Round Robin bajo concurrencia ✅

**Estado:** ✅ Completada  
**Responsable:** @Elpaipsz  
**Rama:** `feature/proxy-core`  
**Prioridad:** `Alta`  
**Dependencias:** HU-E4-03, HU-E4-04, HU-E4-07  
**Etiquetas:** `proxy` `testing` `round-robin` `concurrencia` `evidencia`

---

**Como** desarrollador del proxy PIBL,  
**quiero** ejecutar y documentar una prueba de estrés con 30+ peticiones concurrentes,  
**para** evidenciar que el Round Robin funciona correctamente bajo carga.

#### Criterios de Aceptación
- [ ] Se envían 30+ peticiones concurrentes usando `ab`, bucle de `curl &` o script Python con threads
- [ ] El log evidencia distribución Round Robin equitativa entre los 3 backends
- [ ] No hay crashes, deadlocks ni respuestas perdidas
- [ ] La distribución es aproximadamente 1/3 por backend (tolerancia ±1 por ciclo)
- [ ] Resultados documentados en la sección "Casos de Prueba" del README

#### Tareas
- [ ] Preparar script o comando de prueba concurrente
- [ ] Ejecutar prueba con los 3 backends activos en AWS
- [ ] Extraer del log la secuencia de backends elegidos y verificar RR
- [ ] Documentar evidencia (tabla o extracto de log) en README

#### Definición de Hecho
Sección "Casos de Prueba" del README incluye evidencia de Round Robin bajo carga concurrente.

---
---

# ÉPICA E5 — Caché + TTL + Config extendida — Rol 3: @NavarroAbraham

---

### [HU-E5-00] Módulo de lookup de caché en el flujo del Proxy (decisión HIT/MISS) ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham (Integrado por @Elpaipsz)  
**Rama:** `feature/aws-cache`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-05, HU-E5-01, HU-E5-02  
**Etiquetas:** `caché` `proxy` `hit` `miss`

---

**Como** responsable del módulo de caché,  
**quiero** implementar la lógica de decisión que el proxy ejecuta antes de reenviar una petición al backend: consultar si el recurso existe en disco y si su TTL es válido,  
**para** que el flujo completo HIT/MISS funcione y el caché cumpla su propósito.

#### Criterios de Aceptación
- [ ] Antes de ejecutar Round Robin, el proxy llama a `cache_lookup(uri)`
- [ ] `cache_lookup` retorna HIT (sirve desde disco) o MISS (va al backend)
- [ ] En MISS: después de recibir la respuesta del backend se llama `cache_store(uri, response)`
- [ ] El log registra explícitamente `[CACHE HIT]` o `[CACHE MISS]` por cada petición
- [ ] HIT: el recurso no genera ninguna petición al backend (verificable en log)
- [ ] Validado: primera petición MISS, segunda petición (dentro del TTL) HIT

#### Tareas
- [x] Definir estructura de metadatos: archivo de recurso + `.meta` con timestamp y TTL
- [x] Implementar `cache_lookup(uri)` — verificar existencia + TTL
- [x] Implementar `cache_store(uri, data, size)` — guardar recurso + metadata
- [x] Integrar `cache_lookup` antes del Round Robin en el flujo del proxy
- [x] Integrar `cache_store` después de recibir respuesta del backend
- [x] Registrar HIT/MISS en el logger

#### Definición de Hecho
Log muestra `[CACHE HIT]` en petición repetida dentro del TTL y `[CACHE MISS]` en primera petición. Demostrable en sustentación.

---

### [HU-E5-01] Caché persistente en disco: almacenar respuestas y servir HIT ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham (Integrado por @Elpaipsz)  
**Rama:** `feature/aws-cache`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E4-07  
**Etiquetas:** `caché` `disco` `persistencia`

---

**Como** responsable del caché,  
**quiero** guardar físicamente en disco las respuestas del backend y servirlas directamente en solicitudes repetidas,  
**para** acelerar las respuestas al cliente y cumplir el requisito de persistencia del enunciado.

#### Criterios de Aceptación
- [ ] Los recursos se almacenan en el directorio donde se ejecuta el PIBL
- [ ] El nombre del archivo en caché se deriva de la URI (ejemplo: hash o reemplazo de `/` por `_`)
- [ ] Una respuesta guardada en caché se sirve correctamente al cliente en la siguiente petición
- [ ] El caché persiste si el proceso PIBL se reinicia (está en disco, no en RAM)

#### Tareas
- [x] Implementar función de generación de nombre de archivo desde URI
- [x] Implementar `cache_store()` — escribir respuesta completa en archivo
- [x] Implementar lectura desde caché y envío al cliente
- [x] Verificar que el directorio de caché existe (crearlo si no)

#### Definición de Hecho
HIT/MISS demostrable. Caché persiste tras reinicio del PIBL. Verificado con logs.

---

### [HU-E5-02] TTL configurable por parámetro (expirar y renovar caché) ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham (Integrado por @Elpaipsz)  
**Rama:** `feature/aws-cache`  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E5-01  
**Etiquetas:** `caché` `ttl`

---

**Como** responsable del caché,  
**quiero** expirar los recursos del caché pasado el TTL configurado y reconsultar al backend,  
**para** evitar que el cliente reciba contenido desactualizado.

#### Criterios de Aceptación
- [ ] El TTL es un parámetro pasado al lanzar la aplicación PIBL
- [ ] Los recursos expirados generan un MISS y se actualizan en disco tras la respuesta del backend
- [ ] El timestamp de almacenamiento se guarda en un archivo `.meta` junto al recurso
- [ ] Expiración verificada con `time(NULL)` contra el timestamp almacenado

#### Tareas
- [x] Guardar timestamp de almacenamiento en archivo `.meta`
- [x] Implementar comparación `time(NULL) - timestamp > TTL` en `cache_lookup()`
- [x] Leer el TTL del archivo de configuración
- [ ] Probar con TTL corto (10 segundos) para verificar la expiración

#### Definición de Hecho
Recurso expirado genera MISS y se renueva en disco. Verificado con logs y TTL corto.

---

### [HU-E5-03] Documentar formato del archivo de configuración (incluye TTL) ✅

**Estado:** ✅ Completada  
**Responsable:** @NavarroAbraham  
**Rama:** `feature/aws-cache`  
**Prioridad:** `Alta`  
**Etiquetas:** `config` `docs`  
**Archivos:** `pibl/config.example`

---

**Como** responsable de la configuración,  
**quiero** definir y documentar el formato final del archivo de configuración del PIBL (puerto, backends y TTL),  
**para** permitir un despliegue reproducible sin necesidad de recompilar.

#### Criterios de Aceptación
- [x] El archivo `config.example` está en la raíz del PIBL con todos los campos documentados
- [x] Incluye: puerto de escucha, lista de backends (IP:puerto), TTL del caché en segundos
- [ ] El README incluye la sección "Archivo de Configuración" con ejemplo completo

#### Tareas
- [x] Definir formato definitivo del archivo de configuración
- [x] Crear `config.example` con comentarios explicativos por campo
- [ ] Documentar en README con ejemplo de uso (@NavarroAbraham)

#### Definición de Hecho
`config.example` publicado y documentado en README.

---
---

# ÉPICA E6 — AWS (EC2 + Security Groups + Despliegue)

---

### [HU-E6-01] Crear instancias EC2 para Proxy y 3 Web Servers + Security Groups

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Prioridad:** `Crítica`  
**Etiquetas:** `aws` `ec2` `security-groups` `infra`

---

**Como** responsable de infraestructura,  
**quiero** desplegar 4 instancias EC2 en AWS (1 proxy + 3 web servers) con los Security Groups correctos,  
**para** cumplir el requisito de despliegue en nube del enunciado.

#### Criterios de Aceptación
- [ ] 4 instancias EC2 creadas y accesibles por SSH
- [ ] Security Group del Proxy: permite tráfico entrante en puerto 80/8080 desde internet y saliente hacia los backends
- [ ] Security Group de los TWS: permite tráfico entrante solo desde la IP del Proxy en el puerto configurado
- [ ] Todas las instancias están en la misma región de AWS
- [ ] Las IPs privadas de los TWS son las usadas en el archivo de configuración del PIBL

#### Tareas
- [ ] Crear instancias EC2 (Amazon Linux 2 o Ubuntu 22.04 recomendado)
- [ ] Crear y asignar Security Groups con las reglas correctas
- [ ] Verificar conectividad SSH a todas las instancias
- [ ] Anotar IPs privadas de los TWS para el archivo de configuración

#### Definición de Hecho
Acceso SSH a las 4 instancias verificado. Puertos accesibles según los SGs configurados.

---

### [HU-E6-02] Guía de despliegue reproducible (paso a paso) en README

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Prioridad:** `Alta`  
**Dependencias:** HU-E6-01  
**Etiquetas:** `docs` `deploy` `aws`

---

**Como** responsable de infraestructura,  
**quiero** documentar los comandos exactos para clonar, compilar y ejecutar el TWS y el PIBL en AWS,  
**para** que un evaluador externo pueda reproducir el despliegue completo de forma autónoma.

#### Criterios de Aceptación
- [ ] La guía incluye: clonar el repositorio, instalar dependencias (`gcc`, `make`), compilar con `make`, ejecutar el TWS y el PIBL con los argumentos correctos
- [ ] Los comandos son copiables directamente (formato "copy/paste")
- [ ] Incluye cómo configurar el archivo de configuración del PIBL
- [ ] Incluye cómo generar los archivos de prueba en los TWS

#### Tareas
- [ ] Redactar sección "Guía de Despliegue" en README
- [ ] Verificar que los comandos funcionan en una instancia EC2 limpia
- [ ] Incluir comandos para generar archivos de prueba (`generar_archivos_prueba.sh`)

#### Definición de Hecho
Sección "Guía de Despliegue" en README con comandos copy/paste verificados en AWS.

---

### [HU-E6-03] Integración E2E en AWS: cliente → proxy → (RR/caché) → TWS

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E3-07, HU-E4-07, HU-E5-02, HU-E6-01  
**Etiquetas:** `e2e` `integración` `aws`

---

**Como** responsable de integración,  
**quiero** ejecutar el flujo completo del sistema en AWS (cliente → proxy → caché → round robin → TWS → proxy → cliente),  
**para** tener el sistema funcional listo para la sustentación.

#### Criterios de Aceptación
- [ ] Una petición desde un browser externo llega al proxy, se balancea hacia un TWS y se recibe la respuesta correcta
- [ ] Segunda petición del mismo recurso es servida desde caché (log muestra `[CACHE HIT]`, sin petición al backend)
- [ ] Primera petición o tras expirar TTL muestra `[CACHE MISS]` y contacta el backend
- [ ] Evidencia de HIT/MISS documentada con extracto de log en README

#### Tareas
- [ ] Desplegar TWS en los 3 EC2 con el mismo contenido web
- [ ] Desplegar PIBL en el EC2 del proxy con configuración apuntando a los 3 TWS
- [ ] Ejecutar prueba E2E completa desde un browser externo
- [ ] Documentar evidencia (logs + capturas) en README

#### Definición de Hecho
Flujo E2E demostrable en AWS. Evidencia de caché HIT/MISS en README. Listo para sustentación.

---
---

# ÉPICA E7 — Casos de Prueba Oficiales del Enunciado

---

### [HU-E7-01] Preparar contenido web para los 4 casos de prueba oficiales

**Estado:** ✅ Completada  
**Responsable:** @Nicolaszj  
**Rama:** `feature/parser-http`  
**Prioridad:** `Alta`  
**Etiquetas:** `testing` `contenido` `casos-de-prueba`  
**Archivos:** `tws/www/caso1/`, `tws/www/caso2/`, `tws/www/caso3/`, `tws/www/caso4/`, `tws/www/img/`

---

**Como** responsable del contenido de prueba,  
**quiero** crear las páginas y recursos web para los 4 casos de prueba oficiales del enunciado,  
**para** demostrar en la sustentación que el servidor maneja todos los escenarios requeridos.

#### Criterios de Aceptación
- [x] Caso 1: Página HTML con hipertextos y una imagen
- [x] Caso 2: Página HTML con hipertextos y múltiples imágenes
- [x] Caso 3: Página HTML que referencia un archivo de ~1 MB
- [x] Caso 4: Página HTML con múltiples archivos (~1 MB en total)
- [x] Script `generar_archivos_prueba.sh` para generar los binarios en el EC2
- [x] Imágenes placeholder en `tws/www/img/` (`logo.png`, `foto1-4.jpg`)
- [ ] Ejecutar `generar_archivos_prueba.sh` en los EC2 para crear los `.bin` (@NavarroAbraham)
- [ ] Replicar el contenido en los 3 servidores TWS de AWS (@NavarroAbraham)

#### Tareas
- [x] Crear `tws/www/caso1/index.html` — 1 imagen
- [x] Crear `tws/www/caso2/index.html` — múltiples imágenes
- [x] Crear `tws/www/caso3/index.html` — enlace a archivo de 1 MB
- [x] Crear `tws/www/caso4/index.html` — múltiples archivos ~256 KB c/u
- [x] Crear `tws/www/index.html` — índice general
- [x] Crear script `generar_archivos_prueba.sh`
- [x] Agregar imágenes a `tws/www/img/` (`logo.png`, `foto1.jpg`–`foto4.jpg`)
- [ ] Ejecutar script en los 3 EC2 (@NavarroAbraham)

#### Definición de Hecho
Los 4 casos de prueba son accesibles y funcionan correctamente en los 3 servidores TWS desplegados en AWS.

---

### [HU-E7-02] Ejecutar y documentar las pruebas oficiales en AWS con evidencia

**Estado:** ⬜ Pendiente  
**Responsable:** @NavarroAbraham  
**Prioridad:** `Crítica`  
**Dependencias:** HU-E6-03, HU-E7-01  
**Etiquetas:** `testing` `evidencia` `aws`

---

**Como** equipo de desarrollo,  
**quiero** correr los 4 casos de prueba oficiales en el despliegue de AWS y documentar los resultados con evidencia concreta,  
**para** demostrar el cumplimiento total de los requisitos del enunciado.

#### Criterios de Aceptación
- [ ] Caso 1 ejecutado: página con 1 imagen carga correctamente en browser
- [ ] Caso 2 ejecutado: página con múltiples imágenes carga correctamente
- [ ] Caso 3 ejecutado: archivo de ~1 MB se descarga completamente (verificar con `md5sum`)
- [ ] Caso 4 ejecutado: múltiples archivos se descargan correctamente
- [ ] Evidencia de cada caso (captura de pantalla o salida de comando) en README
- [ ] Extracto de logs del proxy y del TWS mostrando las peticiones

#### Tareas
- [ ] Ejecutar los 4 casos desde un browser real apuntando al proxy
- [ ] Verificar integridad de archivos grandes con `md5sum`
- [ ] Capturar logs del proxy y TWS durante las pruebas
- [ ] Documentar cada caso en la sección "Casos de Prueba" del README

#### Definición de Hecho
Sección "Casos de Prueba" del README completa con evidencia real de los 4 casos en AWS.

---
