#include <string.h>
#include "range.h"

// ---- Bitset primitives (file-local) ----

static void  r_set(Range* r, int i) { r->bits[i >> 5] |=  (1u << (i & 31)); }
static void  r_clr(Range* r, int i) { r->bits[i >> 5] &= ~(1u << (i & 31)); }
static bool  r_tst(const Range* r, int i) { return (bool)((r->bits[i >> 5] >> (i & 31)) & 1u); }

// ---- Construction ----

Range range_empty(void) { Range r; memset(&r, 0, sizeof(r)); return r; }

Range range_full(void) {
	Range r = range_empty();
	for (int i = 0; i < COMBO_COUNT; i++) r_set(&r, i);
	return r;
}

// ---- Operations ----

void range_add(Range* r, Combo c)             { r_set(r, combo_index(c)); }
void range_remove(Range* r, Combo c)          { r_clr(r, combo_index(c)); }
bool range_contains(const Range* r, Combo c)  { return r_tst(r, combo_index(c)); }

int range_count(const Range* r) {
	int n = 0;
	for (int i = 0; i < COMBO_RANGE_WORDS; i++) n += __builtin_popcount(r->bits[i]);
	return n;
}

void range_remove_blocked(Range* r, uint64_t dead) {
	uint64_t d = dead;
	while (d) {
		int bit = __builtin_ctzll(d);
		d &= d - 1;
		Combo combos[51];
		card_combos(make_card(bit), combos);
		for (int i = 0; i < 51; i++)
			range_remove(r, combos[i]);
	}
}

// ---- Filters ----

Range rangefilter_by_rank(const Range* r, uint64_t board, HandRank rank) {
	Range result = range_empty();
	for (int w = 0; w < COMBO_RANGE_WORDS; w++) {
		uint32_t word = r->bits[w];
		while (word) {
			int bit = __builtin_ctz(word);
			word &= word - 1;
			int idx = w * 32 + bit;
			if (idx >= COMBO_COUNT) break;
			Combo c = combo_from_index(idx);
			if (evaluate_cards(board, combo_toBitmask(c)) == rank)
				r_set(&result, idx);
		}
	}
	return result;
}

Range rangefilter_by_draw(const Range* r, uint64_t board, uint8_t draw_flags) {
	Range result = range_empty();
	for (int w = 0; w < COMBO_RANGE_WORDS; w++) {
		uint32_t word = r->bits[w];
		while (word) {
			int bit = __builtin_ctz(word);
			word &= word - 1;
			int idx = w * 32 + bit;
			if (idx >= COMBO_COUNT) break;
			Combo c = combo_from_index(idx);
			DrawInfo d = compute_draws(board, combo_toBitmask(c));
			if (d.flags & draw_flags)
				r_set(&result, idx);
		}
	}
	return result;
}
