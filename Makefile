TARGET = mandelbrot

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O3
LDFLAGS = -lm
RM = rm -f

.PHONY: all $(TARGET) clean

all: $(TARGET)

$(TARGET): mandelbrot.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(TARGET).exe
