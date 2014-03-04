TARGET = mandelbrot

CC = gcc
CFLAGS = -O3 -Wall -Wextra
LDFLAGS = -lm
RM = rm -f

.PHONY: all $(TARGET) clean

all: $(TARGET)

$(TARGET): mandelbrot.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(TARGET).exe
