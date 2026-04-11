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

// Flood every cell of a StateField with a single state; useful for
// initialising before a selective override pass.
void hmap_state_fill(StateField* f, ComboState fill) {
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++) {
			f->grid[row][col] = fill;
		}
	}
}

// Collapse a RangeField to a StateField by picking the modal ComboState
// (the bucket with the highest count) for each cell.  Ties are broken in
// favour of the lower-valued ComboState enum (COMBO_AHEAD wins ties).
// Empty cells (combo_total == 0) are written as COMBO_BEHIND_DEAD so
// renderers can distinguish "not in range" from genuine behind combos.
StateField* hmap_project_state(const RangeField* rf, StateField* out) {
	for (int r = 0; r < HMAP_DIM; r++) {
		for (int c = 0; c < HMAP_DIM; c++) {
			const HMapCell* cell = &rf->grid[r][c];
			if (cell->combo_total == 0) {
				out->grid[r][c] = COMBO_BEHIND_DEAD;
				continue;
			}
			ComboState dominant = COMBO_AHEAD;
			for (int s = 1; s < COMBO_STATE_COUNT; s++) {
				if (cell->statecounts[s] > cell->statecounts[dominant])
					dominant = (ComboState)s;
			}
			out->grid[r][c] = dominant;
		}
	}
	return out;
}

// Axis ↔ rank: ACE (rank 12) → axis 0, TWO (rank 0) → axis 12.
// Higher rank = lower axis, so the field reads ace-first top-left.
static inline int rank_toaxis(int r)    { return 12 - r; }
static inline int axis_torank(int axis) { return 12 - axis; }

// hmap_tocoords: place ht in the standard hand matrix.
//   pair    → diagonal        (row == col == hi_axis)
//   suited  → above diagonal  (row = hi_axis, col = lo_axis  →  row < col)
//   offsuit → below diagonal  (row = lo_axis, col = hi_axis  →  row > col)
void hmap_tocoords(HandType ht, int* row, int* col) {
	int hi_axis = rank_toaxis(handtype_hi(ht));
	int lo_axis = rank_toaxis(handtype_lo(ht));

	if (handtype_is_pair(ht)) {
		*row = hi_axis;
		*col = hi_axis;
		return;
	}

	if (handtype_is_suited(ht)) {
		*row = hi_axis;  // higher rank → lower axis → upper rows
		*col = lo_axis;
		return;
	}

	// offsuit: swap row/col relative to suited so the two live on
	// opposite sides of the diagonal for the same rank pair
	*row = lo_axis;
	*col = hi_axis;
}

// hmap_fromcoords: inverse of hmap_tocoords.
//   row == col  → pair   (single rank r1)
//   row <  col  → suited (r1 > r2 because lower axis = higher rank)
//   row >  col  → offsuit
HandType hmap_fromcoords(int row, int col) {
	assert(row >= 0 && row < HMAP_DIM);
	assert(col >= 0 && col < HMAP_DIM);

	uint8_t r1 = (uint8_t)axis_torank(row);
	uint8_t r2 = (uint8_t)axis_torank(col);

	if (row == col) return make_pair(r1);
	if (row <  col) return make_suited(r1, r2);   // r1 = high, r2 = low
	return make_offsuit(r2, r1);                  // r2 = high, r1 = low
}
