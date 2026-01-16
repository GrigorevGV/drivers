CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2
TARGET=main

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

.PHONY: all clean

