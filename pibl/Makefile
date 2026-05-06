# Makefile para el PIBL (Proxy Inverso + Balanceador)

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = -pthread

OBJS = main.o config.o round_robin.o logger.o http_proxy.o http_parser.o cache.o
TARGET = pibl

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
