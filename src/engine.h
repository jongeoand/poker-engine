#ifndef ENGINE_H_
#define ENGINE_H_

#include <stdlib.h>
#include "card.h"

#ifdef __BMI2__
#include <immintrin.h>
static uint64_t pick_kth_bit(uint64_t mask, int k) {
	return _pdep_u64(1ULL << k, mask);
}
#else
static uint64_t pick_kth_bit(uint64_t mask, int k) {
	uint64_t tmp = mask;
	for (int i = 0; i < k; i++) tmp &= tmp - 1;
	return tmp & -tmp;
}
#endif

static const uint64_t FULL_DECK = ((1ULL << 52) - 1);

uint64_t merge(uint64_t board, uint64_t holecards);

uint64_t get_straight_bit(uint64_t rankmask);

uint64_t rank_occupancy(uint64_t hand);

uint64_t deal(uint64_t* deck);

#define SUIT_MASK 0x1FFFULL

#define DIAMOND_SHIFT 13
#define HEART_SHIFT 26
#define SPADE_SHIFT 39

typedef struct SuitMasks {
	uint64_t clubs;
	uint64_t diamonds;
	uint64_t hearts;
	uint64_t spades;
} SuitMasks;

SuitMasks collapse_suits(uint64_t hand);

typedef struct PairMasks {
	uint64_t exact_pair;
	uint64_t exact_trips;
	uint64_t quads;
} PairMasks;

PairMasks get_pair_masks(SuitMasks suits);

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

bool has_flush(uint64_t hand);
bool has_straight(uint64_t hand);
bool has_straight_flush(uint64_t hand);

bool has_flush_draw(uint64_t hand);
bool has_straight_draw(uint64_t hand);

HandRank detect_pairs(uint64_t hand);
HandRank classify_hand(uint64_t hand);
HandRank evaluate_cards(uint64_t board, uint64_t holecards);

const char* hand_rank_str(HandRank r);

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

HandFeatures analyze_hand(uint64_t hand);

uint32_t calculate_hand_strength(uint64_t hand);

int compare_hands(uint64_t board, uint64_t hero, uint64_t villain);

#define DRAW_FLUSH_DRAW        (1u << 0)
#define DRAW_OESD              (1u << 1)
#define DRAW_GUTSHOT           (1u << 2)
#define DRAW_COMBO_DRAW        (1u << 3)
#define DRAW_BACKDOOR_FLUSH    (1u << 4)
#define DRAW_BACKDOOR_STRAIGHT (1u << 5)

typedef struct {
	uint8_t flags;
	uint8_t flush_outs;
	uint8_t straight_outs;
} DrawInfo;

typedef struct {
	uint64_t direct_outs;
	int direct_count;
	int runner_runner_count;
} Outs;

DrawInfo compute_draws(uint64_t board, uint64_t hand);

Outs enumerate_outs(uint64_t board, uint64_t hero, uint64_t villain);

#endif
