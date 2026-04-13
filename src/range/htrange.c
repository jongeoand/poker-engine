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
