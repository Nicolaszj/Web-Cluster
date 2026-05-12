# Web-Cluster — PIBL-WS

Proyecto académico: Proxy Inverso + Balanceador de Carga + Web Server.

| Módulo | Responsable | Rama |
|--------|-------------|------|
| TWS — Telematics Web Server | @Nicolaszj | `feature/parser-http` |
| PIBL — Proxy + Balanceador | @Elpaipsz | `feature/proxy-core` |
| Infra + Caché + Config | @NavarroAbraham | `feature/aws-cache` |

---

## Video
[https://youtu.be/G3UxJI-tmr8](https://youtu.be/G3UxJI-tmr8)

## TWS — Telematics Web Server

Servidor HTTP/1.1 implementado en C con POSIX sockets. Soporta GET, HEAD y POST con concurrencia por hilos (`pthread`).

### Compilar y ejecutar

```bash
cd tws
make
./server 8080 access.log ./www
```

Antes de la primera ejecución, generar los archivos binarios de prueba:

```bash
bash tws/www/generar_archivos_prueba.sh
```

### Casos de prueba

| Caso | URL | Descripción |
|------|-----|-------------|
| 1 | `http://IP:8080/caso1/index.html` | Página con 1 imagen |
| 2 | `http://IP:8080/caso2/index.html` | Página con 5 imágenes |
| 3 | `http://IP:8080/caso3/index.html` | Archivo binario ~1 MB |
| 4 | `http://IP:8080/caso4/index.html` | 4 archivos ~256 KB c/u |

---

## Casos de Prueba — Evidencia (HU-E3-08)

Ejecutar desde el directorio `tws/` con el servidor activo:

```bash
cd tws
bash test_tws.sh localhost 8080
```

### TEST 1 — GET con 200 OK y Content-Type correcto

```bash
curl -v http://localhost:8080/
```

Respuesta esperada:
```
< HTTP/1.1 200 OK
< Content-Type: text/html; charset=utf-8
< Content-Length: [N]
< Connection: close
```

### TEST 2 — HEAD sin body (RFC 2616 sec. 9.4)

```bash
curl -v -I http://localhost:8080/
```

Respuesta esperada: headers idénticos al GET, **sin body**.

```bash
# Verificar que el body está realmente vacío:
curl -s -X HEAD http://localhost:8080/ | wc -c
# Resultado esperado: 0
```

### TEST 3 — POST con confirmación JSON

```bash
curl -v -X POST -d "nombre=Nicolas&rol=TWS" http://localhost:8080/datos
```

Respuesta esperada:
```
< HTTP/1.1 200 OK
< Content-Type: application/json
{"status":"received","mensaje":"Datos procesados OK"}
```

### TEST 4 — POST sin Content-Length → 400

```bash
curl -v -X POST -H "Content-Length: " http://localhost:8080/datos
```

Respuesta esperada: `HTTP/1.1 400 Bad Request`

### TEST 5 — 404 para recurso inexistente

```bash
curl -v http://localhost:8080/pagina-que-no-existe.html
```

Respuesta esperada: `HTTP/1.1 404 Not Found`

### TEST 6 — Content-Type por extensión

```bash
curl -I http://localhost:8080/img/logo.png    # → image/png
curl -I http://localhost:8080/img/foto1.jpg   # → image/jpeg
curl -I http://localhost:8080/caso1/index.html # → text/html
```

### TEST 7 — Archivo de 1 MB sin corrupción (integridad)

```bash
# En el servidor EC2, tras generar los archivos de prueba:
ORIG=$(md5sum tws/www/files/archivo_1mb.bin | awk '{print }')
curl -s -o /tmp/descarga.bin http://localhost:8080/files/archivo_1mb.bin
DESC=$(md5sum /tmp/descarga.bin | awk '{print }')
[ "$ORIG" = "$DESC" ] && echo "INTEGRO" || echo "CORRUPTO"
```

### TEST 8 — Seguridad: Path Traversal bloqueado

```bash
curl -v "http://localhost:8080/../etc/passwd"
```

Respuesta esperada: `HTTP/1.1 400 Bad Request`

### TEST 9 — Prueba de concurrencia (30 peticiones paralelas)

```bash
for i in $(seq 1 30); do curl -s -o /dev/null http://localhost:8080/ & done; wait
echo "Verificar access.log: debe tener 30 entradas"
```

---

## PIBL — Proxy Inverso + Balanceador de Carga

Ver código en `pibl/`. Escucha en el puerto configurado en `pibl/pibl.conf` y distribuye las peticiones en Round-Robin entre los backends TWS.

```bash
cd pibl
make
./pibl pibl.conf
```

---

## Estructura del repositorio

```
Web-Cluster/
├── tws/                    # Telematics Web Server (Rol 1)
│   ├── server.c            # Entry point — socket + hilos
│   ├── http_parser.c/h     # Parser HTTP/1.1
│   ├── http_response.c/h   # Generador de respuestas
│   ├── mime.c/h            # Tabla de MIME types
│   ├── logger.c/h          # Logger thread-safe
│   ├── Makefile
│   ├── test_tws.sh         # Script de pruebas funcionales
│   └── www/                # DocumentRoot
│       ├── index.html
│       ├── caso1-4/        # Páginas de los 4 casos de prueba
│       ├── img/            # Imágenes (logo.png, foto1-4.jpg)
│       ├── files/          # Binarios (generar con el script)
│       └── generar_archivos_prueba.sh
├── pibl/                   # Proxy Inverso + Balanceador (Rol 2)
│   ├── main.c
│   ├── http_proxy.c/h
│   ├── round_robin.c/h
│   ├── cache.c/h
│   ├── config.c/h
│   ├── logger.c/h
│   └── Makefile
├── BACKLOG_COMPLETO.md
├── CONTRIBUTING.md
├── capturar_evidencia_aws.sh   # Script de evidencia E2E en AWS
└── README.md
```

---

## Despliegue en AWS (HU-E6-01 / E6-02 / E6-03)

### Arquitectura

```
Internet → [EC2 Proxy :8000] → Round Robin → [EC2 TWS-1 :9001]
                                           → [EC2 TWS-2 :9002]
                                           → [EC2 TWS-3 :9003]
```

### 1. Preparar cada instancia EC2 (repetir en las 4)

```bash
sudo apt-get update && sudo apt-get install -y gcc make git   # Ubuntu
# o: sudo yum install -y gcc make git                         # Amazon Linux

git clone https://github.com/Nicolaszj/Web-Cluster.git
cd Web-Cluster
```

### 2. Compilar TWS (en las 3 instancias backend)

```bash
cd tws
make
bash www/generar_archivos_prueba.sh   # genera los .bin de ~1 MB
```

### 3. Levantar los 3 backends TWS

```bash
# En EC2 TWS-1:
./server 9001 tws_9001.log ./www

# En EC2 TWS-2:
./server 9002 tws_9002.log ./www

# En EC2 TWS-3:
./server 9003 tws_9003.log ./www
```

### 4. Configurar y levantar el proxy PIBL (en EC2 Proxy)

Editar `pibl/config.txt` con las IPs privadas reales de los TWS:

```
port=8000
ttl=60
backend=<IP_PRIVADA_TWS1>:9001
backend=<IP_PRIVADA_TWS2>:9002
backend=<IP_PRIVADA_TWS3>:9003
```

```bash
cd pibl
make
./pibl_proxy config.txt
```

### 5. Verificar funcionamiento E2E

```bash
# Desde cualquier máquina con la IP pública del proxy:
curl http://<IP_PUBLICA_PROXY>:8000/caso1/index.html   # debe responder 200
curl http://<IP_PUBLICA_PROXY>:8000/caso1/index.html   # segunda vez: CACHE HIT
```

### 6. Capturar evidencia automática (HU-E7-02)

Ejecutar desde el EC2 del proxy (con el cluster corriendo):

```bash
bash capturar_evidencia_aws.sh <IP_PUBLICA_PROXY> 8000
git add evidencia_aws.txt
git commit -m "test(aws): evidencia pruebas E2E en AWS"
git push origin main
```

### Security Groups requeridos

| Instancia | Puerto | Origen |
|-----------|--------|--------|
| EC2 Proxy | 8000 (TCP) | 0.0.0.0/0 (internet) |
| EC2 Proxy | 22 (SSH) | tu IP |
| EC2 TWS-1/2/3 | 9001-9003 (TCP) | IP privada del Proxy |
| EC2 TWS-1/2/3 | 22 (SSH) | tu IP |
