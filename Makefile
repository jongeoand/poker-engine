CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Isrc -Isrc/core -Isrc/engine -Isrc/range -Isrc/sim -Isrc/cli
LDFLAGS = -lpthread -lm

TARGET      = poker
TEST_TARGET = poker_test

.PHONY: all test clean

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) -o $@ src/main.c $(LDFLAGS)

$(TEST_TARGET): src/tests/runner.c
	$(CC) $(CFLAGS) -o $@ src/tests/runner.c $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)
