#ifndef CARD_H

#define CARD_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	CLUB = 0,
	DIAMOND,
	HEART,
	SPADE
} Suit;

typedef enum {
	TWO = 0,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN,
	JACK,
	QUEEN,
	KING,
	ACE
} Rank;

typedef struct {
	uint8_t suit : 4;
	uint8_t rank : 4;
} Card;

// Card functions
const char* get_suit_string(Suit suit);
const char* get_rank_string(Rank rank);

// Parsing primitives (inverse of get_rank_string / get_suit_string)
// Return the enum value, or -1 on invalid input. Case-insensitive for suits.
int  rank_from_char(char c);
int  suit_from_char(char c);
char rank_to_char(uint8_t rank);

// Construct a card from a bit-position index (0–51): suit = index/13, rank = index%13
Card make_card(int index);

// Bitmask translation
uint64_t toBitmask(Card card);
Card     toCard(uint64_t cardbit);

typedef struct {
	Card a;
	Card b;
} Combo;

// Fill out[51] with every Combo that contains card. Always produces exactly 51 entries.
void card_combos(Card card, Combo out[51]);

// Bitmask
uint64_t combo_toBitmask(Combo c);

bool combo_is_suited(Combo c);
bool combo_is_pair(Combo c);
uint8_t combo_high_rank(Combo c);
uint8_t combo_low_rank(Combo c);

int   combo_index(Combo c);
Combo combo_from_index(int idx);

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
