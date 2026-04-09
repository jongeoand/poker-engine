CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Isrc -Isrc/core -Isrc/engine -Isrc/range -Isrc/sim -Isrc/cli
LDFLAGS = -lpthread -lm

TARGET = poker

.PHONY: all clean

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) -o $@ src/main.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
