#include "equity.h"

// P(at least one hit in k draws from n cards with `outs` favourable cards).
// Uses the complementary miss probability to avoid combinatorial blowup.
static double hypergeometric_hit(int n, int outs, int k) {
	if (outs <= 0 || k <= 0) return 0.0;
	if (outs >= n)           return 1.0;
	double p_miss = 1.0;
	for (int i = 0; i < k; i++)
		p_miss *= (double)(n - outs - i) / (double)(n - i);
	return 1.0 - p_miss;
}

// Sample `needed` cards uniformly at random from `remaining` and run a
// showdown. Called N times by equity_fast for the Monte Carlo path.
static double equity_monte_carlo(uint64_t board, uint64_t hero,
                                 uint64_t villain, int n_samples) {
	uint64_t dead      = board | hero | villain;
	uint64_t remaining = FULL_DECK & ~dead;
	int      n_rem     = __builtin_popcountll(remaining);
	int      needed    = 5 - (int)__builtin_popcountll(board);

	double wins = 0.0;
	for (int s = 0; s < n_samples; s++) {
		uint64_t deck   = remaining;
		uint64_t runout = board;
		for (int i = 0; i < needed; i++) {
			uint64_t card = pick_kth_bit(deck, rand() % (n_rem - i));
			deck   &= ~card;
			runout |=  card;
		}
		int r = compare_hands(runout, hero, villain);
		wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
	}
	return wins / n_samples;
}

double equity_fast(uint64_t board, uint64_t hero, uint64_t villain) {
	int needed = 5 - (int)__builtin_popcountll(board);
	if (needed <= 2)
		return calculate_equity(board, hero, villain);
	return equity_monte_carlo(board, hero, villain, 10000);
}

// Approximate equity via DrawInfo outs counts + hypergeometric probability.
// Identifies the trailing player by strength, reads their flush + straight
// draw outs, and computes P(hit). Intentionally limited: does not capture
// pair improvement, kicker outs, or runner-runner without a draw.
// Use as a study reference alongside equity_fast, not as a replacement.
double equity_from_draws(uint64_t board, uint64_t hero, uint64_t villain) {
	int needed = 5 - (int)__builtin_popcountll(board);
	if (needed == 0) {
		int r = compare_hands(board, hero, villain);
		return r == 1 ? 1.0 : (r == 0 ? 0.5 : 0.0);
	}

	uint64_t dead     = board | hero | villain;
	int      n        = 52 - (int)__builtin_popcountll(dead);
	uint32_t hero_str = calculate_hand_strength(board | hero);
	uint32_t vill_str = calculate_hand_strength(board | villain);

	bool     hero_behind = (hero_str < vill_str);
	uint64_t trailing    = hero_behind ? hero : villain;

	DrawInfo d    = compute_draws(board, trailing);
	int      outs = (int)d.flush_outs + (int)d.straight_outs;

	double trailing_eq = hypergeometric_hit(n, outs, needed);
	return hero_behind ? trailing_eq : 1.0 - trailing_eq;
}

double calculate_equity(uint64_t board, uint64_t hero, uint64_t villain) {
	int cards_needed = 5 - __builtin_popcountll(board);

	if (cards_needed == 0) {
		int r = compare_hands(board, hero, villain);
		return r == 1 ? 1.0 : (r == 0 ? 0.5 : 0.0);
	}

	uint64_t dead      = board | hero | villain;
	uint64_t remaining = FULL_DECK & ~dead;

	double wins  = 0.0;
	int    total = 0;

	uint64_t l1 = remaining;
	while (l1) {
		uint64_t c1 = l1 & -l1;
		l1 &= l1 - 1;
		if (cards_needed == 1) {
			int r = compare_hands(board | c1, hero, villain);
			wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
			total++;
			continue;
		}
		uint64_t l2 = l1;
		while (l2) {
			uint64_t c2 = l2 & -l2;
			l2 &= l2 - 1;
			if (cards_needed == 2) {
				int r = compare_hands(board | c1 | c2, hero, villain);
				wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
				total++;
				continue;
			}
			uint64_t l3 = l2;
			while (l3) {
				uint64_t c3 = l3 & -l3;
				l3 &= l3 - 1;
				if (cards_needed == 3) {
					int r = compare_hands(board | c1 | c2 | c3, hero, villain);
					wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
					total++;
					continue;
				}
				uint64_t l4 = l3;
				while (l4) {
					uint64_t c4 = l4 & -l4;
					l4 &= l4 - 1;
					if (cards_needed == 4) {
						int r = compare_hands(board | c1 | c2 | c3 | c4, hero, villain);
						wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
						total++;
						continue;
					}
					uint64_t l5 = l4;
					while (l5) {
						uint64_t c5 = l5 & -l5;
						l5 &= l5 - 1;
						int r = compare_hands(board | c1 | c2 | c3 | c4 | c5, hero, villain);
						wins += (r == 1) ? 1.0 : (r == 0) ? 0.5 : 0.0;
						total++;
					}
				}
			}
		}
	}

	return total > 0 ? wins / (double)total : 0.0;
}
