CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Isrc
LDFLAGS = -lpthread -lm

TARGET = poker

.PHONY: all clean

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) -o $@ src/main.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
