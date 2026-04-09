#include "card.h"

// -------------- Card
// Returns a pointer to a char that represents the Suit case
const char* get_suit_string(Suit suit) {
	switch (suit) {
		case CLUB:
			return "c";
		case DIAMOND:
			return "d";
		case HEART:
			return "h";
		case SPADE:
			return "s";
		default:
			__builtin_unreachable();
	}

	return "0";
}

// Returns a pointer to a char that represents the Rank of face cards
const char* get_rank_string(Rank rank) {
	switch (rank) {
		case JACK:
			return "J";
		case QUEEN:
			return "Q";
		case KING:
			return "K";
		case ACE:
			return "A";
		default:
			__builtin_unreachable();
	}

	return "0";
}

int rank_from_char(char c) {
	switch (c) {
		case '2':           return TWO;
		case '3':           return THREE;
		case '4':           return FOUR;
		case '5':           return FIVE;
		case '6':           return SIX;
		case '7':           return SEVEN;
		case '8':           return EIGHT;
		case '9':           return NINE;
		case 'T': case 't': return TEN;
		case 'J': case 'j': return JACK;
		case 'Q': case 'q': return QUEEN;
		case 'K': case 'k': return KING;
		case 'A': case 'a': return ACE;
		default:            return -1;
	}
}

int suit_from_char(char c) {
	switch (c) {
		case 'c': case 'C': return CLUB;
		case 'd': case 'D': return DIAMOND;
		case 'h': case 'H': return HEART;
		case 's': case 'S': return SPADE;
		default:            return -1;
	}
}

char rank_to_char(uint8_t rank) {
	if (rank <= NINE) return '2' + rank;
	switch (rank) {
		case TEN:   return 'T';
		case JACK:  return 'J';
		case QUEEN: return 'Q';
		case KING:  return 'K';
		default:    return 'A';
	}
}

Card make_card(int index) {
	return (Card){ .suit = index / 13, .rank = index % 13 };
}

// Converts a Card to a unique 64bit int. Each of the 52 cards is one of the 64 bits (12 bits of padding unused)
uint64_t toBitmask(Card card) {
	uint64_t bitmask = 0;

	int shift = card.suit * 13;

	bitmask |= (1ULL << (shift + card.rank));

	return bitmask;
}

Card toCard(uint64_t cardbit) {
	int bit = __builtin_ctzll(cardbit);
	return (Card){ .suit = bit / 13, .rank = bit % 13 };
}

void card_combos(Card card, Combo out[51]) {
	int card_idx = card.suit * 13 + card.rank;
	int n = 0;
	for (int i = 0; i < 52; i++) {
		if (i == card_idx) continue;
		out[n++] = (Combo){ card, make_card(i) };
	}
}

// -------------- Combos
uint64_t combo_toBitmask(Combo c) {	return toBitmask(c.a) | toBitmask(c.b); }

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


// -------------- HandType 
bool handtype_is_pair(HandType ht)    { return ht.r_a == ht.r_b; }
bool handtype_is_suited(HandType ht)  { return ht.r_b >  ht.r_a; }
bool handtype_is_offsuit(HandType ht) { return ht.r_a >  ht.r_b; }

// Construction
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
	if(handtype_is_pair(ht)) return 6;
	if(handtype_is_suited(ht)) return 4;
	return 12;
}

// ---- HandType indexing ----

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
