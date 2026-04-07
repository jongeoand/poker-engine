#ifndef TESTS_H_
#define TESTS_H_

#include <stdio.h>

#include "card.h"
#include "engine.h"
#include "range.h"
#include "output.h"
#include "game.h"

// --- Automated tests ---
void print_struct_sizes(void);
void test_cards(void);
void test_combos(void);
void test_handtypes(void);
void test_range(void);
void test_handtype_range(void);

void test(void);
void test_alt(void);
#endif
