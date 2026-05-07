# Makefile para el Proxy PIBL
# Proyecto: PIBL-WS | Rol 2: @Nicolaszj

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
TARGET = pibl_proxy

# Archivos fuente
SRCS = main.c cache.c config.c

# Archivos objeto
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf cache/

.PHONY: all clean
