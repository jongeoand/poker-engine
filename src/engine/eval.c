#include "eval.h"

bool has_flush(uint64_t hand) {
	SuitMasks suits = collapse_suits(hand);
	return __builtin_popcountll(suits.clubs)    >= 5
	    || __builtin_popcountll(suits.diamonds) >= 5
	    || __builtin_popcountll(suits.hearts)   >= 5
	    || __builtin_popcountll(suits.spades)   >= 5;
}

bool has_straight(uint64_t hand) {
	return get_straight_bit(rank_occupancy(hand)) != 0;
}

bool has_straight_flush(uint64_t hand) {
	SuitMasks suits = collapse_suits(hand);
	return get_straight_bit(suits.clubs)    != 0
	    || get_straight_bit(suits.diamonds) != 0
	    || get_straight_bit(suits.hearts)   != 0
	    || get_straight_bit(suits.spades)   != 0;
}

const char* hand_rank_str(HandRank r) {
	switch (r) {
		case UNPAIRED:      return "HIGH CARD";
		case PAIRED:        return "PAIR";
		case TWOPAIR:       return "TWO PAIR";
		case TRIPS:         return "TRIPS";
		case STRAIGHT:      return "STRAIGHT";
		case FLUSH:         return "FLUSH";
		case FULL:          return "FULL HOUSE";
		case QUADS:         return "QUADS";
		case STRAIGHTFLUSH: return "STRAIGHT FLUSH";
	}
	return "UNKNOWN";
}

// ----- Static helpers for feature-based classification -----

static HandRank detect_pairs_from_features(const HandFeatures* f) {
	int pairs  = __builtin_popcountll(f->pairs.exact_pair);
	int trips  = __builtin_popcountll(f->pairs.exact_trips);
	int quads_ = __builtin_popcountll(f->pairs.quads);

	if (quads_ >= 1)              return QUADS;
	if (trips >= 1 && pairs >= 1) return FULL;
	if (trips >= 2)               return FULL;
	if (trips >= 1)               return TRIPS;
	if (pairs >= 2)               return TWOPAIR;
	if (pairs >= 1)               return PAIRED;
	return UNPAIRED;
}

static HandRank classify_from_features(const HandFeatures* f) {
	if (f->is_straight_flush) return STRAIGHTFLUSH;
	HandRank paired = detect_pairs_from_features(f);
	if (paired < FULL) {
		if (f->is_flush)    return FLUSH;
		if (f->is_straight) return STRAIGHT;
	}
	return paired;
}

// ----- Single-pass hand analysis -----

HandFeatures analyze_hand(uint64_t hand) {
	HandFeatures f;
	f.suits              = collapse_suits(hand);
	f.suit_counts[CLUB]    = (uint8_t)__builtin_popcountll(f.suits.clubs);
	f.suit_counts[DIAMOND] = (uint8_t)__builtin_popcountll(f.suits.diamonds);
	f.suit_counts[HEART]   = (uint8_t)__builtin_popcountll(f.suits.hearts);
	f.suit_counts[SPADE]   = (uint8_t)__builtin_popcountll(f.suits.spades);
	f.pairs              = get_pair_masks(f.suits);
	f.rank_mask          = (uint16_t)rank_occupancy(hand);
	f.is_flush           = f.suit_counts[CLUB]    >= 5
	                    || f.suit_counts[DIAMOND] >= 5
	                    || f.suit_counts[HEART]   >= 5
	                    || f.suit_counts[SPADE]   >= 5;
	f.is_straight        = get_straight_bit((uint64_t)f.rank_mask) != 0;
	f.is_straight_flush  = get_straight_bit(f.suits.clubs)    != 0
	                    || get_straight_bit(f.suits.diamonds) != 0
	                    || get_straight_bit(f.suits.hearts)   != 0
	                    || get_straight_bit(f.suits.spades)   != 0;
	return f;
}

// ----- Public classification using features -----

HandRank detect_pairs(uint64_t hand) {
	HandFeatures f = analyze_hand(hand);
	return detect_pairs_from_features(&f);
}

HandRank classify_hand(uint64_t hand) {
	HandFeatures f = analyze_hand(hand);
	return classify_from_features(&f);
}

HandRank evaluate_cards(uint64_t board, uint64_t holecards) {
	return classify_hand(merge(board, holecards));
}

// ----- Hand strength and comparison -----

static uint32_t pack_kickers(uint64_t rankmask, int n, int start_bit) {
	uint32_t result = 0;
	for (int i = 0; i < n && rankmask; i++) {
		int r = 63 - __builtin_clzll(rankmask);
		result |= (uint32_t)r << (start_bit - i * 4 - 3);
		rankmask ^= (1ULL << r);
	}
	return result;
}

static int straight_high_rank(uint64_t sbit) {
	int k = 63 - __builtin_clzll(sbit);
	return (k == 0) ? 3 : k + 3;
}

uint32_t calculate_hand_strength(uint64_t hand) {
	HandFeatures f        = analyze_hand(hand);
	HandRank     category = classify_from_features(&f);

	uint64_t sbit     = get_straight_bit((uint64_t)f.rank_mask);
	uint32_t strength = (uint32_t)category << 28;

	switch (category) {
		case UNPAIRED: {
			strength |= pack_kickers((uint64_t)f.rank_mask, 5, 27);
			break;
		}
		case PAIRED: {
			int pair_rank = 63 - __builtin_clzll(f.pairs.exact_pair);
			strength |= (uint32_t)pair_rank << 24;
			strength |= pack_kickers((uint64_t)f.rank_mask ^ (1ULL << pair_rank), 3, 23);
			break;
		}
		case TWOPAIR: {
			int high_pair = 63 - __builtin_clzll(f.pairs.exact_pair);
			int low_pair  = 63 - __builtin_clzll(f.pairs.exact_pair ^ (1ULL << high_pair));
			strength |= (uint32_t)high_pair << 24;
			strength |= (uint32_t)low_pair  << 20;
			strength |= pack_kickers((uint64_t)f.rank_mask ^ (1ULL << high_pair) ^ (1ULL << low_pair), 1, 19);
			break;
		}
		case TRIPS: {
			int trips_rank = 63 - __builtin_clzll(f.pairs.exact_trips);
			strength |= (uint32_t)trips_rank << 24;
			strength |= pack_kickers((uint64_t)f.rank_mask ^ (1ULL << trips_rank), 2, 23);
			break;
		}
		case STRAIGHT: {
			strength |= (uint32_t)straight_high_rank(sbit) << 24;
			break;
		}
		case FLUSH: {
			uint64_t flush_mask = 0;
			if      (f.suit_counts[CLUB]    >= 5) flush_mask = f.suits.clubs;
			else if (f.suit_counts[DIAMOND] >= 5) flush_mask = f.suits.diamonds;
			else if (f.suit_counts[HEART]   >= 5) flush_mask = f.suits.hearts;
			else                                   flush_mask = f.suits.spades;
			strength |= pack_kickers(flush_mask, 5, 27);
			break;
		}
		case FULL: {
			int trips_rank = 63 - __builtin_clzll(f.pairs.exact_trips);
			strength |= (uint32_t)trips_rank << 24;
			int pair_rank = (__builtin_popcountll(f.pairs.exact_trips) >= 2)
				? 63 - __builtin_clzll(f.pairs.exact_trips ^ (1ULL << trips_rank))
				: 63 - __builtin_clzll(f.pairs.exact_pair);
			strength |= (uint32_t)pair_rank << 20;
			break;
		}
		case QUADS: {
			int quads_rank = 63 - __builtin_clzll(f.pairs.quads);
			strength |= (uint32_t)quads_rank << 24;
			strength |= pack_kickers((uint64_t)f.rank_mask ^ (1ULL << quads_rank), 1, 23);
			break;
		}
		case STRAIGHTFLUSH: {
			uint64_t sf_sbit = get_straight_bit(f.suits.clubs);
			if (!sf_sbit) sf_sbit = get_straight_bit(f.suits.diamonds);
			if (!sf_sbit) sf_sbit = get_straight_bit(f.suits.hearts);
			if (!sf_sbit) sf_sbit = get_straight_bit(f.suits.spades);
			strength |= (uint32_t)straight_high_rank(sf_sbit) << 24;
			break;
		}
	}

	return strength;
}

int compare_hands(uint64_t board, uint64_t hero, uint64_t villain) {
	uint32_t hero_strength    = calculate_hand_strength(merge(board, hero));
	uint32_t villain_strength = calculate_hand_strength(merge(board, villain));

	if (hero_strength == villain_strength) return 0;
	return hero_strength > villain_strength ? 1 : -1;
}
