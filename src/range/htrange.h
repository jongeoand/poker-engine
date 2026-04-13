#ifndef HTRANGE_H
#define HTRANGE_H

#include "range.h"
#include "handtype.h"
#include "eval.h"
#include "draws.h"

#define HT_RANGE_WORDS 6  // ceil(HANDTYPE_COUNT / 32); top 23 bits of last word unused

typedef struct {
	uint32_t bits[HT_RANGE_WORDS];
} HandTypeRange;

// ---- Construction ----
HandTypeRange htr_empty(void);
HandTypeRange htr_full(void);

// ---- Membership ----
bool htr_contains(const HandTypeRange* h, HandType ht);
void htr_add(HandTypeRange* h, HandType ht);
void htr_remove(HandTypeRange* h, HandType ht);

// ---- Counts ----
int htr_count(const HandTypeRange* h);           // # of live hand types
int htr_count_pairs(const HandTypeRange* h);
int htr_count_suited(const HandTypeRange* h);
int htr_count_offsuit(const HandTypeRange* h);
int htr_combo_count_max(const HandTypeRange* h); // upper bound: no dead cards
int htr_combo_count_exact(const HandTypeRange* h, uint64_t dead);

// ---- Set operations ----
HandTypeRange htr_union(const HandTypeRange* a, const HandTypeRange* b);
HandTypeRange htr_intersect(const HandTypeRange* a, const HandTypeRange* b);
HandTypeRange htr_subtract(const HandTypeRange* a, const HandTypeRange* b);

// ---- Conversion ----
// Materialize: expand each live hand type into specific combos, filtered by dead.
// dead = board | hero_holecards (any card whose presence blocks combos).
Range         htr_materialize(const HandTypeRange* h, uint64_t dead);
// Compress: lossy — marks a type's bit if any of its combos appear in r.
HandTypeRange range_compress(const Range* r);

#endif
