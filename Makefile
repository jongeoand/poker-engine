CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Isrc -Isrc/core -Isrc/engine -Isrc/range -Isrc/sim -Isrc/cli
LDFLAGS = -lpthread -lm

TARGET         = poker
TEST_TARGET    = poker_test
VISUAL_TARGET  = poker_visual

.PHONY: all test visual clean

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) -o $@ src/main.c $(LDFLAGS)

$(TEST_TARGET): src/tests/runner.c
	$(CC) $(CFLAGS) -o $@ src/tests/runner.c $(LDFLAGS)

$(VISUAL_TARGET): src/tests/visual_runner.c
	$(CC) $(CFLAGS) -o $@ src/tests/visual_runner.c $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

visual: $(VISUAL_TARGET)
	./$(VISUAL_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(VISUAL_TARGET)
