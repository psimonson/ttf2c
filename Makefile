CC=gcc
CFLAGS=-std=c89 -Wall -Wextra -Wno-unused-parameter
CFLAGS+=$(shell pkg-config freetype2 --cflags)
LDFLAGS=$(shell pkg-config freetype2 --libs) -lm
TARGET=ttf2c

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.c.o)

.PHONY: all clean
all: $(TARGET)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *~ $(OBJS) $(TARGET)
