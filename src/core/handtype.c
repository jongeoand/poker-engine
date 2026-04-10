#include "handtype.h"

bool handtype_is_pair(HandType ht)    { return ht.r_a == ht.r_b; }
bool handtype_is_suited(HandType ht)  { return ht.r_b >  ht.r_a; }
bool handtype_is_offsuit(HandType ht) { return ht.r_a >  ht.r_b; }

// implicit categorization in nibble
uint8_t handtype_hi(HandType ht) { return handtype_is_suited(ht) ? ht.r_b : ht.r_a }
uint8_t handtype_lo(HandType ht) { return handtype_is_suited(ht) ? ht.r_a : ht.r_b }

HandType make_pair(uint8_t rank) {
	return (HandType){ .r_a = rank, .r_b = rank };
}

HandType make_suited(uint8_t high, uint8_t low) {
	return (HandType){ .r_a = low, .r_b = high };  // r_b > r_a → suited
}

HandType make_offsuit(uint8_t high, uint8_t low) {
	return (HandType){ .r_a = high, .r_b = low };  // r_a > r_b → offsuit
}

HandType combo_to_handtype(Combo c) {
	uint8_t ra = c.a.rank, rb = c.b.rank;
	if (ra == rb) return make_pair(ra);
	uint8_t high = ra > rb ? ra : rb;
	uint8_t low  = ra < rb ? ra : rb;
	return (c.a.suit == c.b.suit) ? make_suited(high, low) : make_offsuit(high, low);
}

int handtype_combo_count(HandType ht) {
	if (handtype_is_pair(ht))   return 6;
	if (handtype_is_suited(ht)) return 4;
	return 12;
}

// Offset within the suited/offsuit block for a given (high, low) rank pair.
// high and low run from 12..1 and high-1..0 respectively.
// Ordered: AK first, then AQ, ..., 32 last.
static int ht_rank_offset(int h, int l) {
	return (13 + h) * (12 - h) / 2 + (h - 1 - l);
}

int handtype_index(HandType ht) {
	if (handtype_is_pair(ht))   return ht.r_a;
	int h = handtype_is_suited(ht) ? ht.r_b : ht.r_a;  // high rank
	int l = handtype_is_suited(ht) ? ht.r_a : ht.r_b;  // low rank
	int off = ht_rank_offset(h, l);
	return handtype_is_suited(ht) ? 13 + off : 91 + off;
}

HandType handtype_from_index(int idx) {
	if (idx < 13) return make_pair((uint8_t)idx);
	int off    = (idx < 91) ? idx - 13 : idx - 91;
	int suited = (idx < 91);
	for (int h = 12; h >= 1; h--) {
		int base = (13 + h) * (12 - h) / 2;
		if (off >= base && off < base + h) {
			int l = h - 1 - (off - base);
			return suited ? make_suited((uint8_t)h, (uint8_t)l)
			              : make_offsuit((uint8_t)h, (uint8_t)l);
		}
	}
	return make_pair(0);  // unreachable
}

int handtype_combos(HandType ht, uint64_t dead, Combo* out) {
	int n = 0;
	if (handtype_is_pair(ht)) {
		uint8_t rank = ht.r_a;
		for (int s1 = 0; s1 < 4; s1++) {
			for (int s2 = s1 + 1; s2 < 4; s2++) {
				Card ca = { .suit = s1, .rank = rank };
				Card cb = { .suit = s2, .rank = rank };
				if ((toBitmask(ca) | toBitmask(cb)) & dead) continue;
				out[n++] = (Combo){ ca, cb };
			}
		}
	} else if (handtype_is_suited(ht)) {
		uint8_t high = ht.r_b, low = ht.r_a;
		for (int s = 0; s < 4; s++) {
			Card ca = { .suit = s, .rank = high };
			Card cb = { .suit = s, .rank = low };
			if ((toBitmask(ca) | toBitmask(cb)) & dead) continue;
			out[n++] = (Combo){ ca, cb };
		}
	} else {
		uint8_t high = ht.r_a, low = ht.r_b;
		for (int s1 = 0; s1 < 4; s1++) {
			for (int s2 = 0; s2 < 4; s2++) {
				if (s1 == s2) continue;
				Card ca = { .suit = s1, .rank = high };
				Card cb = { .suit = s2, .rank = low };
				if ((toBitmask(ca) | toBitmask(cb)) & dead) continue;
				out[n++] = (Combo){ ca, cb };
			}
		}
	}
	return n;
}
