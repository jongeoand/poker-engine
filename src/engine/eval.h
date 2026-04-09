#ifndef EVAL_H
#define EVAL_H

#include "engine.h"

typedef enum {
	UNPAIRED = 0,
	PAIRED,
	TWOPAIR,
	TRIPS,
	STRAIGHT,
	FLUSH,
	FULL,
	QUADS,
	STRAIGHTFLUSH
} HandRank;

// Single-pass analysis of a hand bitmask — compute once, reuse everywhere.
typedef struct HandFeatures {
	uint16_t rank_mask;       // 13-bit rank occupancy (all suits collapsed)
	uint8_t  suit_counts[4];  // cards per suit, indexed by Suit enum
	SuitMasks suits;          // raw per-suit bitmasks (for flush kickers, SF)
	PairMasks pairs;          // pair/trips/quads bitmasks
	bool is_flush;
	bool is_straight;
	bool is_straight_flush;
} HandFeatures;

bool has_flush(uint64_t hand);
bool has_straight(uint64_t hand);
bool has_straight_flush(uint64_t hand);

HandRank detect_pairs(uint64_t hand);
HandRank classify_hand(uint64_t hand);
HandRank evaluate_cards(uint64_t board, uint64_t holecards);

const char* hand_rank_str(HandRank r);

HandFeatures analyze_hand(uint64_t hand);

uint32_t calculate_hand_strength(uint64_t hand);

int compare_hands(uint64_t board, uint64_t hero, uint64_t villain);

#endif
