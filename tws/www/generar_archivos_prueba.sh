#!/bin/bash
# generar_archivos_prueba.sh (Versión Limpia)

DEST="./tws/www/files"
IMG="./tws/www/img"

mkdir -p "$DEST" "$IMG"

echo "Generando archivos de prueba..."

# Caso 3: un archivo de exactamente 1 MB
dd if=/dev/urandom of="$DEST/archivo_1mb.bin" bs=1024 count=1024 2>/dev/null
echo "  [OK] archivo_1mb.bin (1 MB)"

# Caso 4: cuatro archivos de 256 KB cada uno (~1 MB total)
for i in 1 2 3 4; do
    dd if=/dev/urandom of="$DEST/parte${i}.bin" bs=1024 count=256 2>/dev/null
    echo "  [OK] parte${i}.bin (256 KB)"
done

# Imagen placeholder básica
if [ ! -f "$IMG/logo.png" ]; then
    printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x18\xd8N\x00\x00\x00\x00IEND\xaeB`\x82' > "$IMG/logo.png"
    echo "  [OK] img/logo.png (placeholder)"
fi

echo "Entorno de archivos preparado."
