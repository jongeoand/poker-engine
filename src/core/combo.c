#include "combo.h"

void card_combos(Card card, Combo out[51]) {
	int card_idx = card.suit * 13 + card.rank;
	int n = 0;
	for (int i = 0; i < 52; i++) {
		if (i == card_idx) continue;
		out[n++] = (Combo){ card, make_card(i) };
	}
}

uint64_t combo_toBitmask(Combo c) { return toBitmask(c.a) | toBitmask(c.b); }

bool    combo_is_suited(Combo c)  { return c.a.suit == c.b.suit; }
bool    combo_is_pair(Combo c)    { return c.a.rank == c.b.rank; }
uint8_t combo_high_rank(Combo c)  { return c.a.rank >= c.b.rank ? c.a.rank : c.b.rank; }
uint8_t combo_low_rank(Combo c)   { return c.a.rank <= c.b.rank ? c.a.rank : c.b.rank; }

int combo_index(Combo c) {
	int b0 = c.a.suit * 13 + c.a.rank;
	int b1 = c.b.suit * 13 + c.b.rank;
	int lo = b0 < b1 ? b0 : b1;
	int hi = b0 < b1 ? b1 : b0;
	return hi * (hi - 1) / 2 + lo;
}

Combo combo_from_index(int idx) {
	int hi = 1;
	while ((hi + 1) * hi / 2 <= idx) hi++;
	int lo = idx - hi * (hi - 1) / 2;
	return (Combo){ make_card(lo), make_card(hi) };
}
