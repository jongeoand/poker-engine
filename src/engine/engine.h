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

#endif
