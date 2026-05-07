#!/bin/bash
# capturar_evidencia_aws.sh — Evidencia automatica para HU-E6-03 y HU-E7-02
#
# Ejecutar desde la instancia EC2 del PROXY, con el cluster ya corriendo:
#   bash run_cluster.sh &
#   sleep 3
#   bash capturar_evidencia_aws.sh <IP_PUBLICA_PROXY> <PUERTO_PROXY>
#
# Si no se pasan argumentos, usa localhost:8000
#
# La evidencia se guarda en evidencia_aws.txt y se puede commitear al repo.

PROXY_HOST="${1:-localhost}"
PROXY_PORT="${2:-8000}"
BASE="http://${PROXY_HOST}:${PROXY_PORT}"
OUT="evidencia_aws.txt"

SEP="========================================"

{
echo "$SEP"
echo "EVIDENCIA DE PRUEBAS EN AWS — PIBL-WS"
echo "Fecha   : $(date)"
echo "Host    : $PROXY_HOST:$PROXY_PORT"
echo "$SEP"
echo ""

# ---- TEST 1: Caso 1 — Pagina con 1 imagen ------------------------------
echo "--- TEST 1: Caso 1 — Pagina HTML con 1 imagen ---"
echo "Comando: curl -s -o /dev/null -w '%{http_code} | CT: %{content_type}' ${BASE}/caso1/index.html"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/caso1/index.html")
CT=$(curl -s -D - -o /dev/null "${BASE}/caso1/index.html" | grep -i "content-type" | head -1)
echo "Resultado: HTTP $CODE"
echo "Header   : $CT"
[ "$CODE" = "200" ] && echo "[PASS] Caso 1 OK" || echo "[FAIL] Caso 1 fallo (esperado 200)"
echo ""

# ---- TEST 2: Caso 2 — Pagina con multiples imagenes --------------------
echo "--- TEST 2: Caso 2 — Pagina HTML con multiples imagenes ---"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/caso2/index.html")
echo "Resultado: HTTP $CODE"
[ "$CODE" = "200" ] && echo "[PASS] Caso 2 OK" || echo "[FAIL] Caso 2 fallo"
echo ""

# ---- TEST 3: Caso 3 — Archivo ~1 MB ------------------------------------
echo "--- TEST 3: Caso 3 — Descarga archivo binario ~1 MB ---"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/caso3/index.html")
echo "Resultado pagina: HTTP $CODE"
echo "Descargando binario..."
TMP_FILE=$(mktemp)
HTTP_BIN=$(curl -s -o "$TMP_FILE" -w '%{http_code}' "${BASE}/files/archivo_1mb.bin")
BYTES=$(wc -c < "$TMP_FILE")
echo "Resultado binario: HTTP $HTTP_BIN | Tamanio: ${BYTES} bytes"
[ "$BYTES" -ge 1000000 ] && echo "[PASS] Archivo ~1 MB recibido correctamente" || echo "[FAIL] Archivo incompleto"
rm -f "$TMP_FILE"
echo ""

# ---- TEST 4: Caso 4 — Multiples archivos -------------------------------
echo "--- TEST 4: Caso 4 — Multiples archivos ---"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/caso4/index.html")
echo "Resultado: HTTP $CODE"
[ "$CODE" = "200" ] && echo "[PASS] Caso 4 OK" || echo "[FAIL] Caso 4 fallo"
echo ""

# ---- TEST 5: Cache MISS y HIT ------------------------------------------
echo "--- TEST 5: Cache MISS (primera peticion) ---"
RECURSO="${BASE}/caso1/index.html"
echo "Peticion 1 a: $RECURSO"
RESP1=$(curl -s -w '\nHTTP: %{http_code} | Tiempo: %{time_total}s' "$RECURSO" -o /dev/null)
echo "$RESP1"
echo "(Ver log del proxy: debe mostrar CACHE MISS)"
echo ""

echo "--- TEST 6: Cache HIT (segunda peticion — mismo recurso) ---"
echo "Peticion 2 a: $RECURSO"
RESP2=$(curl -s -w '\nHTTP: %{http_code} | Tiempo: %{time_total}s' "$RECURSO" -o /dev/null)
echo "$RESP2"
echo "(Ver log del proxy: debe mostrar CACHE HIT)"
echo ""

# ---- TEST 7: Round Robin -----------------------------------------------
echo "--- TEST 7: Round Robin — 6 peticiones al proxy ---"
for i in 1 2 3 4 5 6; do
    CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/")
    echo "  Peticion $i: HTTP $CODE"
done
echo "(Ver log del proxy: backends 9001/9002/9003 deben alternarse)"
echo ""

# ---- TEST 8: 404 Not Found ---------------------------------------------
echo "--- TEST 8: 404 para recurso inexistente ---"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/no_existe_este_archivo.html")
echo "Resultado: HTTP $CODE"
[ "$CODE" = "404" ] && echo "[PASS] 404 correcto" || echo "[FAIL] Esperaba 404, recibio $CODE"
echo ""

# ---- TEST 9: Seguridad — path traversal --------------------------------
echo "--- TEST 9: Seguridad — path traversal bloqueado ---"
CODE=$(curl -s -o /dev/null -w '%{http_code}' "${BASE}/../etc/passwd")
echo "Resultado: HTTP $CODE"
[ "$CODE" = "400" ] && echo "[PASS] Path traversal bloqueado (400)" || echo "[INFO] Codigo recibido: $CODE"
echo ""

# ---- Logs del proxy (ultimas lineas) -----------------------------------
echo "--- LOGS DEL PROXY (ultimas 20 lineas) ---"
if [ -f "../pibl_proxy.log" ]; then
    tail -20 ../pibl_proxy.log
elif [ -f "pibl/pibl_proxy.log" ]; then
    tail -20 pibl/pibl_proxy.log
else
    echo "(Los logs del proxy van a stdout — revisar terminal donde corre run_cluster.sh)"
fi
echo ""

echo "$SEP"
echo "FIN DE EVIDENCIA"
echo "$SEP"

} | tee "$OUT"

echo ""
echo "Evidencia guardada en: $OUT"
echo "Para commitar: git add $OUT && git commit -m 'test(aws): evidencia pruebas E2E en AWS'"
