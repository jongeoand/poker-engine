#ifndef HTRANGE_H
#define HTRANGE_H

#include "range.h"
#include "handtype.h"
#include "eval.h"
#include "draws.h"

#define HT_RANGE_WORDS 6  // ceil(HANDTYPE_COUNT / 32); top 23 bits of last word unused

typedef struct {
	uint32_t bits[HT_RANGE_WORDS];
} HandTypeRange;

// ---- Construction ----
HandTypeRange htr_empty(void);
HandTypeRange htr_full(void);

// ---- Membership ----
bool htr_contains(const HandTypeRange* h, HandType ht);
void htr_add(HandTypeRange* h, HandType ht);
void htr_remove(HandTypeRange* h, HandType ht);

// ---- Counts ----
int htr_count(const HandTypeRange* h);           // # of live hand types
int htr_count_pairs(const HandTypeRange* h);
int htr_count_suited(const HandTypeRange* h);
int htr_count_offsuit(const HandTypeRange* h);
int htr_combo_count_max(const HandTypeRange* h); // upper bound: no dead cards
int htr_combo_count_exact(const HandTypeRange* h, uint64_t dead);

// ---- Set operations ----
HandTypeRange htr_union(const HandTypeRange* a, const HandTypeRange* b);
HandTypeRange htr_intersect(const HandTypeRange* a, const HandTypeRange* b);
HandTypeRange htr_subtract(const HandTypeRange* a, const HandTypeRange* b);

// ---- Conversion ----
// Materialize: expand each live hand type into specific combos, filtered by dead.
// dead = board | hero_holecards (any card whose presence blocks combos).
Range         htr_materialize(const HandTypeRange* h, uint64_t dead);
// Compress: lossy — marks a type's bit if any of its combos appear in r.
HandTypeRange range_compress(const Range* r);

// ---- Filters ----
HandTypeRange htrfilter_by_rank(const HandTypeRange* htr, uint64_t board, HandRank rank);
HandTypeRange htrfilter_by_draw(const HandTypeRange* htr, uint64_t board, uint8_t draw_flags);

// ---- Approximate board profile (hand-type level) ----
// For each live type, picks a representative combo and classifies it vs board.
// Approximate: ignores suit-specific blocking within a type.
typedef struct {
	int made[9];  // count of types making each HandRank (PAIRED..STRAIGHTFLUSH)
	int draw;     // UNPAIRED types with at least one draw
	int missed;   // UNPAIRED types with no draws
	int total;    // live types (those with >= 1 combo unblocked by board)
} HtrBoardProfile;

HtrBoardProfile htr_board_profile(const HandTypeRange* h, uint64_t board);

// ---- Equity split vs a specific hero combo ----
// Partitions `villains` by whether a representative combo of each type
// beats (behind), loses to (ahead), or ties (chopping) the hero's hand.
// hero  = combo_toBitmask(hero_combo)
// dead  = board | hero (blocks villain representative combos automatically)
// Approximate: one representative combo per type — accurate for rank/kicker
// comparisons, may mis-classify on suit-dependent boards (e.g. monotone).
typedef struct {
	HandTypeRange ahead;    // hero beats representative villain combo
	HandTypeRange behind;   // villain beats hero
	HandTypeRange chopping; // tie
} HtrEquitySplit;

HtrEquitySplit htr_vs_combo(uint64_t board, uint64_t hero, const HandTypeRange* villains);

// Convenience wrappers returning a single bucket.
HandTypeRange htrfilter_ahead(uint64_t board, uint64_t hero, const HandTypeRange* villains);
HandTypeRange htrfilter_behind(uint64_t board, uint64_t hero, const HandTypeRange* villains);

#endif
