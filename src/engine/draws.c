#include "draws.h"

bool has_flush_draw(uint64_t hand) {
	SuitMasks suits = collapse_suits(hand);
	return __builtin_popcountll(suits.clubs)    == 4
	    || __builtin_popcountll(suits.diamonds) == 4
	    || __builtin_popcountll(suits.hearts)   == 4
	    || __builtin_popcountll(suits.spades)   == 4;
}

bool has_straight_draw(uint64_t hand) {
	if (has_straight(hand)) return false;
	uint64_t ext = rank_occupancy(hand);
	ext = (ext << 1) | ((ext >> 12) & 1);
	for (int i = 0; i <= 9; i++) {
		if (__builtin_popcountll(ext & (0x1FULL << i)) == 4) return true;
	}
	return false;
}

static uint64_t rank_full_mask(int r) {
	return (1ULL << r) | (1ULL << (r + 13)) | (1ULL << (r + 26)) | (1ULL << (r + 39));
}

static int ext_bit_to_rank(int p) {
	if (p == 0) return 12;
	return p - 1;
}

DrawInfo compute_draws(uint64_t board, uint64_t hole_cards) {
	uint64_t hand  = merge(board, hole_cards);
	HandFeatures f = analyze_hand(hand);
	DrawInfo d     = {0};

	if (!f.is_flush) {
		for (int s = 0; s < 4; s++) {
			if      (f.suit_counts[s] == 4) { d.flags |= DRAW_FLUSH_DRAW;     d.flush_outs += (uint8_t)(13 - f.suit_counts[s]); }
			else if (f.suit_counts[s] == 3) { d.flags |= DRAW_BACKDOOR_FLUSH; }
		}
	}

	if (!f.is_straight) {
		uint64_t ext         = ((uint64_t)f.rank_mask << 1) | (((uint64_t)f.rank_mask >> 12) & 1);
		uint64_t hole_rank   = rank_occupancy(hole_cards);
		uint64_t hole_ext    = (hole_rank << 1) | ((hole_rank >> 12) & 1);
		uint64_t out_rank_mask = 0;

		for (int i = 0; i <= 9; i++) {
			uint64_t window      = 0x1FULL << i;
			uint64_t window_bits = ext & window;
			int cnt = __builtin_popcountll(window_bits);

			if (cnt == 4) {
				uint64_t lo4 = 0xFULL <<  i;
				uint64_t hi4 = 0xFULL << (i + 1);

				if (window_bits == lo4 || window_bits == hi4) {
					d.flags |= DRAW_OESD;

					if (window_bits == lo4) {
						int rank_above = i + 3;
						if (rank_above <= 12)
							out_rank_mask |= (1ULL << rank_above);
						if (i >= 1) {
							int rank_below = ext_bit_to_rank(i - 1);
							if (rank_below >= 0 && rank_below <= 12)
								out_rank_mask |= (1ULL << rank_below);
						}
					} else {
						int rank_below = ext_bit_to_rank(i);
						if (rank_below >= 0 && rank_below <= 12)
							out_rank_mask |= (1ULL << rank_below);
						if (i + 5 <= 13) {
							int rank_above = i + 4;
							if (rank_above <= 12)
								out_rank_mask |= (1ULL << rank_above);
						}
					}
				} else {
					d.flags |= DRAW_GUTSHOT;
					uint64_t missing = window & ~window_bits;
					int p    = __builtin_ctzll(missing);
					int rank = ext_bit_to_rank(p);
					if (rank >= 0 && rank <= 12)
						out_rank_mask |= (1ULL << rank);
				}
			} else if (cnt == 3) {
				if (hole_ext & window)
					d.flags |= DRAW_BACKDOOR_STRAIGHT;
			}
		}

		uint64_t tmp = out_rank_mask;
		while (tmp) {
			int r = __builtin_ctzll(tmp);
			tmp &= tmp - 1;
			d.straight_outs += (uint8_t)(4 - __builtin_popcountll(hand & rank_full_mask(r)));
		}
	}

	if ((d.flags & (DRAW_FLUSH_DRAW | DRAW_BACKDOOR_FLUSH)) &&
	    (d.flags & (DRAW_OESD | DRAW_GUTSHOT)))
		d.flags |= DRAW_COMBO_DRAW;

	return d;
}

Outs enumerate_outs(uint64_t board, uint64_t hero, uint64_t villain) {
	Outs outs = {0};

	uint64_t dead      = board | hero | villain;
	uint64_t remaining = FULL_DECK & ~dead;

	uint64_t tmp = remaining;
	while (tmp) {
		uint64_t card = tmp & -tmp;
		tmp &= tmp - 1;
		if (compare_hands(board | card, hero, villain) == 1) {
			outs.direct_outs |= card;
			outs.direct_count++;
		}
	}

	if (__builtin_popcountll(board) == 3) {
		uint64_t outer = remaining;
		while (outer) {
			uint64_t c1 = outer & -outer;
			outer &= outer - 1;
			if (outs.direct_outs & c1) continue;

			uint64_t inner = outer;
			while (inner) {
				uint64_t c2 = inner & -inner;
				inner &= inner - 1;
				if (outs.direct_outs & c2) continue;
				if (compare_hands(board | c1 | c2, hero, villain) == 1)
					outs.runner_runner_count++;
			}
		}
	}

	return outs;
}
