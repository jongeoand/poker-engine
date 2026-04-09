#ifndef DRAWS_H
#define DRAWS_H

#include "engine.h"
#include "eval.h"

bool has_flush_draw(uint64_t hand);
bool has_straight_draw(uint64_t hand);

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
