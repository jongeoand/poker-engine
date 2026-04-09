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

// 1 byte representation of a card
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

#endif
