#!/bin/bash
# generar_archivos_prueba.sh
# Genera los archivos binarios necesarios para los casos de prueba 3 y 4.
# Ejecutar UNA sola vez en el servidor EC2 antes de iniciar el TWS.
#
# Uso: bash generar_archivos_prueba.sh

set -e

DEST="$(dirname "$0")/files"
IMG="$(dirname "$0")/img"

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

# Imagen de marcador de posicion para los casos 1 y 2
# (reemplazar con imagenes reales para la sustentacion)
if [ ! -f "$IMG/logo.png" ]; then
    # Crear un PNG minimo valido de 1x1 pixel rojo (sin herramientas externas)
    printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x18\xd8N\x00\x00\x00\x00IEND\xaeB`\x82' > "$IMG/logo.png"
    echo "  [OK] img/logo.png (placeholder 1x1px)"
fi

echo ""
echo "Listo. Ahora ejecuta el servidor con:"
echo "  ./server 8080 access.log ./www"
