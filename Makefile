TARGET = mandelbrot

CC = gcc
CFLAGS = -Wall -Wextra
RM = rm -f

.PHONY: all $(TARGET) clean

all: $(TARGET)

$(TARGET): mandelbrot.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(TARGET).exe
