#include "card.h"

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
