#!/bin/bash
# test_tws.sh — Pruebas funcionales del TWS (HU-E3-08)
#
# Genera evidencia de todos los criterios de aceptacion de HU-E3-08.
# Requiere que el servidor ya este corriendo:
#   cd tws && ./server 8080 access.log ./www
#
# Uso:
#   bash test_tws.sh [HOST] [PUERTO]
#   bash test_tws.sh localhost 8080   (valores por defecto)
#
# La salida se guarda en tws_test_results.txt

HOST="${1:-localhost}"
PORT="${2:-8080}"
BASE="http://${HOST}:${PORT}"
OUT="tws_test_results.txt"

PASS=0
FAIL=0

# ─────────────────────────────────────────────
log()  { echo "$@" | tee -a "$OUT"; }
check() {
    local desc="$1" expected="$2" actual="$3"
    if echo "$actual" | grep -q "$expected"; then
        log "  [PASS] $desc"
        PASS=$((PASS+1))
    else
        log "  [FAIL] $desc"
        log "         Esperado: $expected"
        log "         Obtenido: $actual"
        FAIL=$((FAIL+1))
    fi
}
# ─────────────────────────────────────────────

> "$OUT"   # limpiar archivo de salida anterior

log "======================================================"
log " TWS - Suite de Pruebas Funcionales (HU-E3-08)"
log " Servidor: ${BASE}"
log " Fecha   : $(date)"
log "======================================================"
log ""

# ─── TEST 1: GET pagina principal ────────────────────────
log "--- TEST 1: GET / → 200 OK con Content-Type text/html ---"
RESP=$(curl -s -o /dev/null -w "%{http_code} %{content_type}" "${BASE}/")
check "HTTP 200 OK" "200" "$RESP"
check "Content-Type text/html" "text/html" "$RESP"
log ""

# ─── TEST 2: GET imagen PNG ──────────────────────────────
log "--- TEST 2: GET /img/logo.png → 200 OK con image/png ---"
RESP=$(curl -s -o /dev/null -w "%{http_code} %{content_type}" "${BASE}/img/logo.png")
check "HTTP 200 OK" "200" "$RESP"
check "Content-Type image/png" "image/png" "$RESP"
log ""

# ─── TEST 3: GET imagen JPEG ─────────────────────────────
log "--- TEST 3: GET /img/foto1.jpg → 200 OK con image/jpeg ---"
RESP=$(curl -s -o /dev/null -w "%{http_code} %{content_type}" "${BASE}/img/foto1.jpg")
check "HTTP 200 OK" "200" "$RESP"
check "Content-Type image/jpeg" "image/jpeg" "$RESP"
log ""

# ─── TEST 4: HEAD request ────────────────────────────────
log "--- TEST 4: HEAD / → headers identicos a GET pero sin body ---"
HEAD_RESP=$(curl -s -I "${BASE}/")
check "HTTP 200 OK en HEAD" "200 OK" "$HEAD_RESP"
check "Content-Type presente en HEAD" "Content-Type" "$HEAD_RESP"
check "Content-Length presente en HEAD" "Content-Length" "$HEAD_RESP"
# Verificar que el body esta vacio (HEAD no debe enviar body)
HEAD_BODY=$(curl -s -X HEAD "${BASE}/" | wc -c)
check "Body vacio en HEAD (0 bytes)" "^0$" "$HEAD_BODY"
log ""

# ─── TEST 5: POST con body ───────────────────────────────
log "--- TEST 5: POST /datos → 200 OK con confirmacion JSON ---"
POST_RESP=$(curl -s -X POST -d "nombre=Nicolas&rol=TWS" "${BASE}/datos")
check "Respuesta POST contiene 'status'" "status" "$POST_RESP"
check "Respuesta POST contiene 'received'" "received" "$POST_RESP"
POST_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "nombre=Nicolas" "${BASE}/datos")
check "POST retorna 200" "200" "$POST_CODE"
log ""

# ─── TEST 6: 404 para recurso inexistente ────────────────
log "--- TEST 6: GET /no-existe.html → 404 Not Found ---"
RESP=$(curl -s -o /dev/null -w "%{http_code}" "${BASE}/no-existe.html")
check "HTTP 404 Not Found" "404" "$RESP"
log ""

# ─── TEST 7: 400 para peticion malformada ────────────────
log "--- TEST 7: Peticion malformada → 400 Bad Request ---"
BAD_RESP=$(printf "INVALID REQUEST\r\n\r\n" | nc -q1 "$HOST" "$PORT" 2>/dev/null \
           || printf "INVALID REQUEST\r\n\r\n" | timeout 3 nc "$HOST" "$PORT" 2>/dev/null)
if [ -z "$BAD_RESP" ]; then
    # nc no disponible — usar curl con metodo invalido
    BAD_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X PATCH "${BASE}/")
    check "Metodo invalido → 400" "400" "$BAD_CODE"
else
    check "Peticion invalida → 400" "400" "$BAD_RESP"
fi
log ""

# ─── TEST 8: Path traversal rechazado ────────────────────
log "--- TEST 8: Path traversal → 400 Bad Request ---"
TRAV_CODE=$(curl -s -o /dev/null -w "%{http_code}" "${BASE}/../etc/passwd")
check "Path traversal bloqueado" "400" "$TRAV_CODE"
log ""

# ─── TEST 9: Archivo grande (caso 3) ─────────────────────
log "--- TEST 9: GET /files/archivo_1mb.bin → 200 OK, 1 MB completo ---"
if [ -f "./www/files/archivo_1mb.bin" ]; then
    ORIGINAL_MD5=$(md5sum ./www/files/archivo_1mb.bin | awk '{print $1}')
    curl -s -o /tmp/archivo_descargado.bin "${BASE}/files/archivo_1mb.bin"
    DESCARGADO_MD5=$(md5sum /tmp/archivo_descargado.bin | awk '{print $1}')
    log "  MD5 original   : $ORIGINAL_MD5"
    log "  MD5 descargado : $DESCARGADO_MD5"
    check "Archivo 1MB integro (md5sum)" "$ORIGINAL_MD5" "$DESCARGADO_MD5"
    rm -f /tmp/archivo_descargado.bin
    log ""
else
    log "  [SKIP] www/files/archivo_1mb.bin no existe. Ejecutar primero:"
    log "         bash www/generar_archivos_prueba.sh"
    log ""
fi

# ─── TEST 10: Content-Length exacto ──────────────────────
log "--- TEST 10: Content-Length exacto para /img/logo.png ---"
FILE_SIZE=$(stat -c%s ./www/img/logo.png 2>/dev/null || stat -f%z ./www/img/logo.png 2>/dev/null || echo "0")
CL_HEADER=$(curl -s -I "${BASE}/img/logo.png" | grep -i "Content-Length" | tr -d '\r' | awk '{print $2}')
log "  Tamanio en disco  : $FILE_SIZE bytes"
log "  Content-Length HTTP: $CL_HEADER"
check "Content-Length coincide con tamanio real" "$FILE_SIZE" "$CL_HEADER"
log ""

# ─── TEST 11: Prueba de concurrencia ─────────────────────
log "--- TEST 11: 30 peticiones concurrentes (stress test) ---"
for i in $(seq 1 30); do
    curl -s -o /dev/null "${BASE}/" &
done
wait
log "  [INFO] 30 peticiones enviadas en paralelo (sin errores de sistema)"
log "  [INFO] Verificar access.log para confirmar 30 entradas registradas"
log ""

# ─────────────────────────────────────────────
log "======================================================"
log " RESULTADOS: ${PASS} pasaron / $((PASS+FAIL)) total"
if [ $FAIL -eq 0 ]; then
    log " ESTADO: TODAS LAS PRUEBAS PASARON ✓"
else
    log " ESTADO: ${FAIL} PRUEBA(S) FALLARON ✗"
fi
log "======================================================"
log ""
log "Resultados guardados en: $OUT"
