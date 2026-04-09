#ifndef RANGE_H_
#define RANGE_H_

#include "combo.h"
#include "eval.h"
#include "draws.h"

#define COMBO_COUNT       1326
#define COMBO_RANGE_WORDS 42  // ceil(COMBO_COUNT / 32)

typedef struct {
	uint32_t bits[COMBO_RANGE_WORDS];
} Range;

Range range_empty(void);
Range range_full(void);

void range_add(Range* r, Combo c);
void range_remove(Range* r, Combo c);
bool range_contains(const Range* r, Combo c);

int  range_count(const Range* r);
void range_remove_blocked(Range* r, uint64_t dead);

// ---- Filters ----

Range rangefilter_by_rank(const Range* r, uint64_t board, HandRank rank);
Range rangefilter_by_draw(const Range* r, uint64_t board, uint8_t draw_flags);

#endif
