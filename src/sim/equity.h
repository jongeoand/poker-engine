#ifndef EQUITY_H_
#define EQUITY_H_

#include <stdint.h>
#include "engine.h"
#include "eval.h"
#include "draws.h"

// Exact equity via full runout enumeration.
// O(C(remaining, cards_needed)) — fast on river/turn, slow preflop.
double calculate_equity(uint64_t board, uint64_t hero, uint64_t villain);

// Exact for river/turn/flop (≤1081 runouts). Monte Carlo (~±1%, 10k samples)
// for preflop and any situation with 3+ cards to come.
double equity_fast(uint64_t board, uint64_t hero, uint64_t villain);

// Approximate equity from flush + straight draw outs via hypergeometric
// probability. Study reference only — does not capture pair/kicker outs.
double equity_from_draws(uint64_t board, uint64_t hero, uint64_t villain);

#endif
