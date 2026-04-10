#ifndef HANDTYPE_H
#define HANDTYPE_H

#include "combo.h"

// Category is implicit in nibble ordering:
//   r_a == r_b  →  pair    (rank = r_a)
//   r_a >  r_b  →  offsuit (high = r_a, low = r_b)
//   r_a <  r_b  →  suited  (high = r_b, low = r_a)
typedef struct {
	uint8_t r_a : 4;
	uint8_t r_b : 4;
} HandType;

#define HANDTYPE_COUNT 169  // 13 pairs + 78 suited + 78 offsuit

bool handtype_is_pair(HandType ht);
bool handtype_is_suited(HandType ht);
bool handtype_is_offsuit(HandType ht);

uint8_t handtype_hi(Handtype ht);
uint8_t handtype_lo(Handtype ht);

// Construction
HandType make_pair(uint8_t rank);
HandType make_suited(uint8_t high, uint8_t low);
HandType make_offsuit(uint8_t high, uint8_t low);

HandType combo_to_handtype(Combo c);

int handtype_combo_count(HandType ht);

// Canonical index [0, 168]:
//   Pairs   0–12:  TWO..ACE (index == rank)
//   Suited 13–90:  AKs=13 .. 32s=90
//   Offsuit 91–168: AKo=91 .. 32o=168
int      handtype_index(HandType ht);
HandType handtype_from_index(int idx);

// Enumerate all combos of ht not blocked by dead.
// out[] must hold at least handtype_combo_count(ht) entries (max 12).
// Returns the number of live combos written.
int handtype_combos(HandType ht, uint64_t dead, Combo* out);

#endif
