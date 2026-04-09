#include "engine.h"

uint64_t merge(uint64_t board, uint64_t holecards) { return board | holecards; }

uint64_t get_straight_bit(uint64_t rankmask) {
	uint64_t ext = (rankmask << 1) | ((rankmask >> 12) & 1);
	return ext & (ext >> 1) & (ext >> 2) & (ext >> 3) & (ext >> 4);
}

uint64_t rank_occupancy(uint64_t hand) {
	return (hand | (hand >> DIAMOND_SHIFT) | (hand >> HEART_SHIFT) | (hand >> SPADE_SHIFT)) & SUIT_MASK;
}

uint64_t deal(uint64_t* deck) {
	int count = __builtin_popcountll(*deck);
	uint64_t card = pick_kth_bit(*deck, rand() % count);
	*deck &= ~card;
	return card;
}

SuitMasks collapse_suits(uint64_t hand) {
	return (SuitMasks){
		.clubs    =  hand                   & SUIT_MASK,
		.diamonds = (hand >> DIAMOND_SHIFT) & SUIT_MASK,
		.hearts   = (hand >> HEART_SHIFT)   & SUIT_MASK,
		.spades   = (hand >> SPADE_SHIFT)   & SUIT_MASK
	};
}

PairMasks get_pair_masks(SuitMasks s) {
	uint64_t at_least_pair = (s.clubs & s.diamonds)
	                       | (s.clubs & s.hearts)
	                       | (s.clubs & s.spades)
	                       | (s.diamonds & s.hearts)
	                       | (s.diamonds & s.spades)
	                       | (s.hearts & s.spades);

	uint64_t at_least_trips = (s.clubs & s.diamonds & s.hearts)
	                        | (s.clubs & s.diamonds & s.spades)
	                        | (s.clubs & s.hearts & s.spades)
	                        | (s.diamonds & s.hearts & s.spades);

	uint64_t quads = s.clubs & s.diamonds & s.hearts & s.spades;

	return (PairMasks){
		.exact_pair  = at_least_pair  & ~at_least_trips,
		.exact_trips = at_least_trips & ~quads,
		.quads       = quads
	};
}
