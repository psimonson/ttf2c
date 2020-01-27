CC=gcc
CFLAGS=-std=c89 -Wall -Wextra -Wno-unused-parameter
TARGET=ttf2c
VERSION=1.0

DEBUG=no
ifeq ($(DEBUG),yes)
CFLAGS+=-g
else
CFLAGS+=-O2
endif
CFLAGS+=$(shell pkg-config sdl2 --cflags)
LDFLAGS=$(shell pkg-config sdl2 --libs)
LDFLAGS+=$(shell pkg-config SDL2_ttf --libs)
LDFLAGS+=-lm

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.c.o)

.PHONY: all dist dist-clean clean
all: $(TARGET)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

dist: dist-clean
	@cd ..; tar cv --exclude=.git $(TARGET) | xz -9 > ../$(TARGET)-$(VERSION).txz

dist-clean: clean

clean:
	rm -f *~ $(OBJS) $(TARGET)
