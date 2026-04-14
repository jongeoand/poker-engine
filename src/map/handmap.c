#include <assert.h>

#include "map/handmap.h"

// Reset a cell to the zero state (all buckets empty, no combos recorded).
void hmap_cell_clear(HMapCell* p) {
	for (int i = 0; i < COMBO_STATE_COUNT; i++) {
		p->statecounts[i] = 0;
	}
	p->combo_total = 0;
}

// A cell is empty when no combos from the active range map to its hand type.
bool hmap_cell_isempty(const HMapCell* p) {
	return p->combo_total == 0;
}

// true if multiple ComboState types are present in this cell
bool hmap_cell_ismixed(const HMapCell* p) {
	int types = 0;
	for (int i = 0; i < COMBO_STATE_COUNT; i++) {
		if (p->statecounts[i] > 0) {
			types++;
			if (types > 1) return true;
		}
	}
	return false;
}

// Record one combo: increment its state bucket and the running total.
void hmap_cell_add(HMapCell* p, ComboState state) {
	p->statecounts[state]++;
	p->combo_total++;
}

// Fold src into dst bucket-by-bucket; used to union two RangeFields.
void hmap_cell_merge(HMapCell* dst, const HMapCell* src) {
	for (int i = 0; i < COMBO_STATE_COUNT; i++) {
		dst->statecounts[i] += src->statecounts[i];
	}
	dst->combo_total += src->combo_total;
}

// Clear every cell so the grid is ready for a fresh build or accumulation.
void hmap_clear(RangeField* f) {
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++) {
			hmap_cell_clear(&f->grid[row][col]);
		}
	}
}

// Build a RangeField by streaming every live combo in `htr`.
// Each combo is classified against the current board/hero state, mapped to its
// hand-matrix cell via hmap_tocoords, and recorded with hmap_cell_add.
RangeField hmap_build(const HandTypeRange* htr, uint64_t dead, uint64_t board, uint64_t hero) {
	RangeField f;
	hmap_clear(&f);

	HtrComboStream stream;
	combostream_init(&stream, htr, dead);

	Combo c;
	while (combostream_next(&stream, &c)) {
		ComboState state = classify_combostate(board, hero, combo_toBitmask(c));
		HandType ht = combo_to_handtype(c);

		int row, col;
		hmap_tocoords(ht, &row, &col);

		hmap_cell_add(&f.grid[row][col], state);
	}

	return f;
}

// Total live combos across the entire grid (sum of all cell combo_totals).
int hmap_total(const RangeField* f) {
	int total = 0;
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++) {
			total += f->grid[row][col].combo_total;
		}
	}
	return total;
}

// Total combos in state `s` across the entire grid.
int hmap_count(const RangeField* f, ComboState s) {
	int total = 0;
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++) {
			total += f->grid[row][col].statecounts[s];
		}
	}
	return total;
}

