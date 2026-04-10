#ifndef RANGE_ITERATE_H
#define RANGE_ITERATE_H

#include <stdbool.h>
#include <stdint.h>

#include "core/combo.h"
#include "core/handtype.h"
#include "range/htrange.h"

typedef struct {
	const HandTypeRange* htr;
	uint64_t dead;

	int ht_index;	// next handtype index to scan
	Combo buf[12];  // exact legal combos for current handtype
	int buf_n;      // number of valid combos in buffer
	int buf_i;      // next combo to return from buffer
} HtrComboStream;

// Create HtrComboStream - initialize .htr and .dead to function params
void combostream_init(HtrComboStream* s, const HandTypeRange* htr, uint64_t dead);

bool combostream_next(HtrComboStream* stream, Combo* out);

typedef void (*ComboFn) (Combo combo, void* context);

void stream_foreach_combo(const HandTypeRange* htr, uint64_t dead, ComboFn fn, void* ctx);

#endif
