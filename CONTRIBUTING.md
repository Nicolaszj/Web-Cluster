# CONTRIBUTING.md — Guía de Contribución

## Proyecto PIBL-WS: Proxy Inverso + Balanceador + Web Server

---

## 1. Estrategia de Ramas

El proyecto usa **tres ramas de desarrollo**, una por rol:

| Rama | Responsable | Módulo |
|------|-------------|--------|
| `feature/parser-http` | @Nicolaszj | TWS — Telematics Web Server |
| `feature/proxy-core` | @Elpaipsz | PIBL — Proxy + Balanceador |
| `feature/aws-cache` | @NavarroAbraham | Infra + Caché + Config + Docs |

La rama `main` es la rama de integración. **Solo se hace merge a `main` cuando el código compila sin errores y pasa las pruebas básicas.**

### Flujo de trabajo

```
feature/parser-http  ──┐
feature/proxy-core   ──┼──► main
feature/aws-cache    ──┘
```

Cada desarrollador trabaja exclusivamente en su rama. Para integrar:
1. Crear un Pull Request desde la rama de feature hacia `main`
2. El PR debe pasar el checklist de revisión (ver sección 3)
3. Requiere revisión de al menos un compañero antes del merge

---

## 2. Restricciones de Implementación

> **Este proyecto es de carácter académico. Todo el código debe escribirse desde cero.**

### Está PROHIBIDO usar:

- Librerías HTTP externas (libcurl, mongoose, libevent, etc.)
- Parsers externos de cualquier tipo
- Frameworks web de cualquier índole
- Código generado o copiado directamente de herramientas de IA
- Código copiado de repositorios externos sin comprenderlo completamente

### Está PERMITIDO usar únicamente:

- Lenguaje C (estándar C99 o C11)
- Biblioteca estándar de C: `stdlib.h`, `stdio.h`, `string.h`, `math.h`, `time.h`
- Sockets y red POSIX: `sys/socket.h`, `netinet/in.h`, `arpa/inet.h`, `unistd.h`
- Hilos POSIX: `pthread.h`
- Sistema de archivos POSIX: `sys/stat.h`, `sys/types.h`, `fcntl.h`, `dirent.h`
- Cualquier syscall estándar POSIX.1-2017

### Compartir código entre compañeros

- **Permitido:** Compartir código a través de **commits en GitHub** (Pull Requests, ramas)
- **Prohibido:** Compartir archivos `.c` / `.h` por WhatsApp, correo, Drive u otros canales externos al repositorio

Toda colaboración debe quedar trazada en el historial de Git.

---

## 3. Checklist Mínimo para Pull Requests

Antes de abrir un PR a `main`, verifica que todos estos ítems estén cumplidos:

### Compilación y calidad de código
- [ ] El código compila en Linux con `gcc -Wall -Wextra -pthread` sin errores ni warnings
- [ ] El código compila en una instancia EC2 limpia (Ubuntu 22.04 / Amazon Linux 2)
- [ ] No hay memoria sin liberar evidente (sin `malloc` huérfanos)
- [ ] No hay file descriptors sin cerrar en flujos normales

### Restricciones
- [ ] ¿El código usa **solo** stdlib + POSIX? (sin librerías externas)
- [ ] ¿Todo el código fue escrito por el autor del PR? (no copiado de IA ni repos externos)

### Funcionalidad
- [ ] La HU que cierra este PR tiene todos sus criterios de aceptación marcados como completados
- [ ] Se ejecutó al menos una prueba manual del flujo implementado
- [ ] Los logs generados son legibles y coherentes

### Documentación
- [ ] El `README.md` fue actualizado si el PR agrega funcionalidad observable
- [ ] Las funciones públicas nuevas tienen comentarios de cabecera en el `.h`

---

## 4. Convención de Commits

Usar el formato:

```
[MÓDULO] Descripción breve en imperativo

Cuerpo opcional explicando el qué y el por qué (no el cómo).
Cierra: #NRO_ISSUE
```

**Ejemplos:**

```
[TWS] Implementar parser HTTP para GET, HEAD y POST

[CACHE] Agregar TTL configurable con archivo .meta

[PROXY] Corregir condición de carrera en Round Robin

[DOCS] Agregar sección Casos de Prueba al README
```

**Módulos válidos:** `TWS`, `PROXY`, `CACHE`, `CONFIG`, `INFRA`, `DOCS`, `TEST`

---

## 5. Estructura del Repositorio

```
/
├── tws/                  # Telematics Web Server (Rol 1)
│   ├── server.c
│   ├── http_parser.c / .h
│   ├── http_response.c / .h
│   ├── mime.c / .h
│   ├── logger.c / .h
│   ├── Makefile
│   └── www/              # DocumentRoot con casos de prueba
│
├── pibl/                 # Proxy Inverso + Balanceador (Rol 2 + 3)
│   ├── main.c
│   ├── config.c / .h
│   ├── round_robin.c / .h
│   ├── cache.c / .h
│   ├── logger.c / .h
│   ├── config.example
│   └── Makefile
│
├── diagramas/            # Diagramas UML (fuente + PNG)
│   ├── deployment.puml / .png
│   ├── secuencia.puml / .png
│   └── componentes.puml / .png
│
├── README.md
└── CONTRIBUTING.md       # Este archivo
```

---

## 6. Configuración Mínima de Entorno

```bash
# Ubuntu 22.04 / Amazon Linux 2
sudo apt update
sudo apt install -y gcc make

# Compilar TWS
cd tws && make

# Compilar PIBL
cd pibl && make

# Ejecutar TWS
./server 8080 server.log ./www

# Ejecutar PIBL
./pibl config.txt
```

---

## 7. Política de Issues y Kanban

- Cada HU del backlog tiene un Issue de GitHub vinculado
- El Issue se mueve en el tablero Kanban conforme avanza: **Backlog → ToDo → In Progress → In Review → Done**
- Al abrir un PR, referenciar el Issue con `Cierra: #NRO` para cerrarlo automáticamente al merge
- No cerrar Issues manualmente sin un PR asociado

---

*Última actualización: Mayo 2026 — @NavarroAbraham*
