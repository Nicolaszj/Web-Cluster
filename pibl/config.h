#ifndef CONFIG_H
#define CONFIG_H

/*
 * config.h — Módulo de configuración del Proxy PIBL
 * Proyecto: PIBL-WS  |  Rol 3: @NavarroAbraham
 * Rama: feature/aws-cache
 *
 * Lee el archivo de configuración y expone la struct Config
 * con todos los parámetros del sistema.
 *
 * Formato del archivo (ver config.example):
 *   port=8080
 *   ttl=60
 *   backend=192.168.1.10:9090
 *   backend=192.168.1.11:9090
 *   backend=192.168.1.12:9090
 *
 * Uso:
 *   Config cfg;
 *   if (config_cargar("config.txt", &cfg) != 0) { exit(1); }
 *   printf("Puerto: %d\n", cfg.puerto);
 *   printf("TTL: %d\n", cfg.ttl);
 *   printf("Backends: %d\n", cfg.num_backends);
 *   config_imprimir(&cfg);
 *   // ... usar cfg ...
 *   config_destruir(&cfg);
 */

/* ---- Límites ---------------------------------------------------------- */

/** Número máximo de backends soportados */
#define CONFIG_MAX_BACKENDS   16

/** Longitud máxima de una IP o hostname */
#define CONFIG_MAX_IP_LEN     64

/** Longitud máxima de la ruta del archivo de configuración */
#define CONFIG_MAX_PATH_LEN  256

/** Longitud máxima de una línea en el archivo de configuración */
#define CONFIG_MAX_LINE_LEN  512

/* ---- Estructuras ------------------------------------------------------ */

/**
 * Representa un servidor backend (IP + puerto).
 */
typedef struct {
    char ip[CONFIG_MAX_IP_LEN]; /**< Dirección IP o hostname del backend */
    int  puerto;                /**< Puerto TCP del backend */
} Backend;

/**
 * Configuración completa del Proxy PIBL.
 * Llenada por config_cargar() y liberada por config_destruir().
 */
typedef struct {
    int     puerto;                          /**< Puerto en que escucha el proxy */
    int     ttl;                             /**< TTL del caché en segundos */
    Backend backends[CONFIG_MAX_BACKENDS];   /**< Lista de backends */
    int     num_backends;                    /**< Número de backends cargados */
} Config;

/* ---- Funciones -------------------------------------------------------- */

/**
 * Carga y parsea el archivo de configuración.
 *
 * El archivo puede tener líneas en cualquier orden.
 * Las líneas que comienzan con '#' o están vacías se ignoran.
 * Los campos desconocidos también se ignoran (permisivo hacia delante).
 *
 * @param ruta  Ruta al archivo de configuración (ej: "config.txt")
 * @param cfg   Puntero a la struct Config a rellenar (no debe ser NULL)
 * @return  0 en éxito, -1 si el archivo no se puede abrir,
 *          -2 si faltan campos obligatorios (port o backends)
 */
int config_cargar(const char *ruta, Config *cfg);

/**
 * Imprime la configuración cargada en stdout.
 * Útil para logs de arranque del proxy.
 *
 * @param cfg  Puntero a una Config previamente cargada
 */
void config_imprimir(const Config *cfg);

/**
 * Libera los recursos de la struct Config.
 * En la implementación actual no hay memoria dinámica,
 * pero está expuesta para permitir extensiones futuras.
 *
 * @param cfg  Puntero a la Config a destruir
 */
void config_destruir(Config *cfg);

#endif /* CONFIG_H */
