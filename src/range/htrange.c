#include <string.h>
#include "htrange.h"

// ---- Bitset primitives (file-local) ----

static void htr_bset(HandTypeRange* h, int i) { h->bits[i >> 5] |=  (1u << (i & 31)); }
static void htr_bclr(HandTypeRange* h, int i) { h->bits[i >> 5] &= ~(1u << (i & 31)); }
static bool htr_btst(const HandTypeRange* h, int i) {
	return (bool)((h->bits[i >> 5] >> (i & 31)) & 1u);
}

// ---- Construction ----

HandTypeRange htr_empty(void) {
	HandTypeRange h;
	memset(&h, 0, sizeof(h));
	return h;
}

HandTypeRange htr_full(void) {
	HandTypeRange h = htr_empty();
	for (int i = 0; i < HANDTYPE_COUNT; i++) htr_bset(&h, i);
	return h;
}

// ---- Membership ----

bool htr_contains(const HandTypeRange* h, HandType ht) { return htr_btst(h, handtype_index(ht)); }
void htr_add(HandTypeRange* h, HandType ht)             { htr_bset(h, handtype_index(ht)); }
void htr_remove(HandTypeRange* h, HandType ht)          { htr_bclr(h, handtype_index(ht)); }

// ---- Counts ----
// Bit ranges: pairs=0..12, suited=13..90, offsuit=91..168

int htr_count_pairs(const HandTypeRange* h) {
	return __builtin_popcount(h->bits[0] & 0x00001FFFu);  // bits 0-12
}

int htr_count_suited(const HandTypeRange* h) {
	return __builtin_popcount(h->bits[0] & 0xFFFFE000u)   // bits 13-31
	     + __builtin_popcount(h->bits[1])                  // bits 32-63
	     + __builtin_popcount(h->bits[2] & 0x07FFFFFFu);  // bits 64-90
}

int htr_count_offsuit(const HandTypeRange* h) {
	return __builtin_popcount(h->bits[2] & 0xF8000000u)   // bits 91-95
	     + __builtin_popcount(h->bits[3])                  // bits 96-127
	     + __builtin_popcount(h->bits[4])                  // bits 128-159
	     + __builtin_popcount(h->bits[5] & 0x000001FFu);  // bits 160-168
}

int htr_count(const HandTypeRange* h) {
	// Last word only has 9 valid bits (160-168); mask out the unused top 23.
	int n = 0;
	for (int i = 0; i < HT_RANGE_WORDS - 1; i++) n += __builtin_popcount(h->bits[i]);
	return n + __builtin_popcount(h->bits[HT_RANGE_WORDS - 1] & 0x000001FFu);
}

int htr_combo_count_max(const HandTypeRange* h) {
	return htr_count_pairs(h)   * 6
	     + htr_count_suited(h)  * 4
	     + htr_count_offsuit(h) * 12;
}

int htr_combo_count_exact(const HandTypeRange* h, uint64_t dead) {
	int total = 0;
	Combo buf[12];
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(h, i)) continue;
		total += handtype_combos(handtype_from_index(i), dead, buf);
	}
	return total;
}

// ---- Set operations ----

HandTypeRange htr_union(const HandTypeRange* a, const HandTypeRange* b) {
	HandTypeRange r;
	for (int i = 0; i < HT_RANGE_WORDS; i++) r.bits[i] = a->bits[i] | b->bits[i];
	return r;
}

HandTypeRange htr_intersect(const HandTypeRange* a, const HandTypeRange* b) {
	HandTypeRange r;
	for (int i = 0; i < HT_RANGE_WORDS; i++) r.bits[i] = a->bits[i] & b->bits[i];
	return r;
}

HandTypeRange htr_subtract(const HandTypeRange* a, const HandTypeRange* b) {
	HandTypeRange r;
	for (int i = 0; i < HT_RANGE_WORDS; i++) r.bits[i] = a->bits[i] & ~b->bits[i];
	return r;
}

// ---- Conversion ----

Range htr_materialize(const HandTypeRange* h, uint64_t dead) {
	Range r = range_empty();
	Combo buf[12];
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(h, i)) continue;
		HandType ht = handtype_from_index(i);
		int n = handtype_combos(ht, dead, buf);
		for (int j = 0; j < n; j++) range_add(&r, buf[j]);
	}
	return r;
}

HandTypeRange range_compress(const Range* r) {
	HandTypeRange h = htr_empty();
	for (int w = 0; w < COMBO_RANGE_WORDS; w++) {
		uint32_t word = r->bits[w];
		while (word) {
			int bit = __builtin_ctz(word);
			word &= word - 1;
			int idx = w * 32 + bit;
			if (idx >= COMBO_COUNT) break;
			htr_bset(&h, handtype_index(combo_to_handtype(combo_from_index(idx))));
		}
	}
	return h;
}

//
// Todo - investigate functionality of htrfilter / htrequitysplit / htr_vs_combo
//
// Initial idea was that HandTypeRange is easier to store across streets but 
// ComboStream gives more detail as it looks specifically at each live combo
//
// ---- Filters - deprecated ----

HandTypeRange htrfilter_by_rank(const HandTypeRange* htr, uint64_t board, HandRank rank) {
	HandTypeRange result = htr_empty();
	Combo buf[12];
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(htr, i)) continue;
		HandType ht = handtype_from_index(i);
		int n = handtype_combos(ht, board, buf);
		if (n == 0) continue;
		if (evaluate_cards(board, combo_toBitmask(buf[0])) == rank)
			htr_bset(&result, i);
	}
	return result;
}

HandTypeRange htrfilter_by_draw(const HandTypeRange* htr, uint64_t board, uint8_t draw_flags) {
	HandTypeRange result = htr_empty();
	Combo buf[12];
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(htr, i)) continue;
		HandType ht = handtype_from_index(i);
		int n = handtype_combos(ht, board, buf);
		if (n == 0) continue;
		DrawInfo d = compute_draws(board, combo_toBitmask(buf[0]));
		if (d.flags & draw_flags)
			htr_bset(&result, i);
	}
	return result;
}

// ---- Equity split vs a specific hero combo ----

HtrEquitySplit htr_vs_combo(uint64_t board, uint64_t hero, const HandTypeRange* villains) {
	HtrEquitySplit s;
	s.ahead    = htr_empty();
	s.behind   = htr_empty();
	s.chopping = htr_empty();
	Combo buf[12];
	uint64_t dead = board | hero;
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(villains, i)) continue;
		HandType ht = handtype_from_index(i);
		int n = handtype_combos(ht, dead, buf);
		if (n == 0) continue;
		int cmp = compare_hands(board, hero, combo_toBitmask(buf[0]));
		if      (cmp > 0) htr_bset(&s.ahead,    i);
		else if (cmp < 0) htr_bset(&s.behind,   i);
		else              htr_bset(&s.chopping,  i);
	}
	return s;
}

HandTypeRange htrfilter_ahead(uint64_t board, uint64_t hero, const HandTypeRange* villains) {
	return htr_vs_combo(board, hero, villains).ahead;
}

HandTypeRange htrfilter_behind(uint64_t board, uint64_t hero, const HandTypeRange* villains) {
	return htr_vs_combo(board, hero, villains).behind;
}

// ---- Approximate board profile ----

HtrBoardProfile htr_board_profile(const HandTypeRange* h, uint64_t board) {
	HtrBoardProfile p;
	memset(&p, 0, sizeof(p));
	Combo buf[12];
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (!htr_btst(h, i)) continue;
		HandType ht = handtype_from_index(i);
		int n = handtype_combos(ht, board, buf);
		if (n == 0) continue;
		p.total++;
		uint64_t mask = combo_toBitmask(buf[0]);
		HandRank rank = evaluate_cards(board, mask);
		if (rank != UNPAIRED) {
			p.made[rank]++;
		} else {
			DrawInfo d = compute_draws(board, mask);
			if (d.flags) p.draw++;
			else         p.missed++;
		}
	}
	return p;
}
