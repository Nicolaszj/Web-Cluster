#!/bin/bash

# Script para lanzar el Cluster de Telematica (Proxy + 3 Backends)
# Proyecto: PIBL-WS

echo "=== Iniciando compilación ==="
cd tws && make clean && make
cd ../pibl && make clean && make
cd ..

echo "=== Limpiando procesos previos ==="
pkill server
pkill pibl_proxy

echo "=== Lanzando 3 Servidores TWS (Backends) ==="
# ./server <PUERTO> <LOG> <RAIZ>
./tws/server 9001 tws_9001.log ./tws/www > /dev/null 2>&1 &
echo "  [OK] TWS en puerto 9001"
./tws/server 9002 tws_9002.log ./tws/www > /dev/null 2>&1 &
echo "  [OK] TWS en puerto 9002"
./tws/server 9003 tws_9003.log ./tws/www > /dev/null 2>&1 &
echo "  [OK] TWS en puerto 9003"

sleep 1

echo "=== Lanzando Proxy PIBL (Puerto 8000) ==="
cd pibl
./pibl_proxy
