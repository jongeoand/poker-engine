#ifndef COMBO_H
#define COMBO_H

#include "card.h"

typedef struct {
	Card a;
	Card b;
} Combo;

// Fill out[51] with every Combo that contains card. Always produces exactly 51 entries.
void card_combos(Card card, Combo out[51]);

// Bitmask
uint64_t combo_toBitmask(Combo c);

bool    combo_is_suited(Combo c);
bool    combo_is_pair(Combo c);
uint8_t combo_high_rank(Combo c);
uint8_t combo_low_rank(Combo c);

int   combo_index(Combo c);
Combo combo_from_index(int idx);

#endif
