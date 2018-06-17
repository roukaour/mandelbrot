TARGET = mandelbrot

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O3
LDFLAGS = -lm
RM = rm -f

.PHONY: all $(TARGET) clean

all: $(TARGET)

$(TARGET).o: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) -c $^

main.o: main.c mandelbrot.h
	$(CC) $(CFLAGS) -c $^

$(TARGET): main.o $(TARGET).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET) $(TARGET).exe *.o
