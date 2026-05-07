# Makefile para el Telematics Web Server (TWS)
#
# Por que Makefile y no un script de bash?
# make rastrea dependencias: si solo cambias http_parser.c, solo recompila
# ese archivo y el binario final. Un script bash recompilaria todo siempre.
# En proyectos grandes eso ahorra mucho tiempo.
#
# Uso:
#   make          â†’ compila el servidor
#   make clean    â†’ borra los objetos y el binario
#   make run      â†’ compila y ejecuta con valores de prueba

CC     = gcc

# Flags de compilacion:
#   -Wall    : activar TODOS los warnings del compilador (obligatorio en la academia)
#   -Wextra  : warnings adicionales no incluidos en -Wall
#   -g       : incluir informacion de debug para usar con gdb o valgrind
#   -std=c11 : usar el estandar C11 (soporta declaraciones mezcladas con codigo)
CFLAGS = -Wall -Wextra -g -std=c11

# -lpthread: enlazar con la libreria de hilos POSIX
# Debe ir AL FINAL del comando de enlace (convencion del linker de gcc)
LIBS   = -lpthread

TARGET = server
SRCS   = server.c http_parser.c http_response.c mime.c logger.c
OBJS   = $(SRCS:.c=.o)

# Regla principal: construir el binario a partir de los .o
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Regla generica: compilar cualquier .c a .o
# $< = el archivo fuente (.c), $@ = el archivo destino (.o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Atajo para probar localmente (ajustar la ruta de DocumentRoot segun necesites)
run: $(TARGET)
	./$(TARGET) 8080 tws_access.log ./www

.PHONY: clean run
