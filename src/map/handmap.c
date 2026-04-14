#include "map/handmap.h"
#include "sim/equity.h"

// Merge src into dst nibble-by-nibble.
// Direct uint64_t addition would carry across nibble boundaries, so each
// (state, suit) sub-count is extracted, summed, and written back individually.
void hmap_cell_merge(HMapCell* dst, HMapCell src) {
	for (int s = 0; s < COMBO_STATE_COUNT; s++) {
		for (int u = 0; u < SUIT_CLASS_COUNT; u++) {
			int val   = hmap_cell_get(*dst, s, u) + hmap_cell_get(src, s, u);
			int shift = HMAP_CELL_SHIFT(s, u);
			*dst = (*dst & ~(0xFULL << shift)) | ((uint64_t)val << shift);
		}
	}
}

// Zero every cell in the grid.
void hmap_clear(RangeField* f) {
	for (int r = 0; r < HMAP_DIM; r++)
		for (int c = 0; c < HMAP_DIM; c++)
			f->grid[r][c] = 0;
}

// Build a RangeField by streaming every live combo in `htr`.
// Each combo is classified on two independent axes:
//   - ComboState: how the combo relates to the hero hand (ahead/chop/behind)
//   - SuitClass:  the combo's strongest flush-draw relationship to the board
// Both are recorded together in the packed HMapCell nibble at (state, suit).
RangeField hmap_build(const HandTypeRange* htr, uint64_t dead, uint64_t board, uint64_t hero) {
	RangeField f;
	hmap_clear(&f);

	HtrComboStream stream;
	combostream_init(&stream, htr, dead);

	Combo c;
	while (combostream_next(&stream, &c)) {
		uint64_t   combo_mask = combo_toBitmask(c);
		ComboState state      = classify_combostate(board, hero, combo_mask);
		SuitClass  suit       = classify_suit(board, combo_mask);

		int row, col;
		hmap_tocoords(combo_to_handtype(c), &row, &col);
		hmap_cell_add(&f.grid[row][col], (int)state, (int)suit);
	}

	return f;
}

// Sum of all combo counts across the entire grid.
int hmap_total(const RangeField* f) {
	int total = 0;
	for (int r = 0; r < HMAP_DIM; r++)
		for (int c = 0; c < HMAP_DIM; c++)
			total += hmap_cell_total(f->grid[r][c]);
	return total;
}

// Total combos in ComboState `s` across all 169 cells (all suit classes combined).
int hmap_count(const RangeField* f, ComboState s) {
	int total = 0;
	for (int r = 0; r < HMAP_DIM; r++)
		for (int c = 0; c < HMAP_DIM; c++)
			total += hmap_cell_state_total(f->grid[r][c], (int)s);
	return total;
}

// Build a ScalarField (mean villain equity) from a RangeField.
//
// For each (hand_type, SuitClass) bucket that has combos, find one canonical
// representative combo and call equity_fast once.  All combos in the bucket
// are suit-isomorphic on the given board and therefore share the same equity.
// The bucket count from the cell drives the weighted average without any
// additional equity calls.
//
// Empty cells (no live combos) are set to the sentinel value -1.0f.
ScalarField scalar_build(const RangeField* rf, uint64_t dead,
                         uint64_t board, uint64_t hero) {
	ScalarField sf;
	for (int r = 0; r < HMAP_DIM; r++)
		for (int c = 0; c < HMAP_DIM; c++)
			sf.grid[r][c] = -1.0f;

	Combo buf[12]; /* max combos per hand type (offsuit: 12) */

	for (int r = 0; r < HMAP_DIM; r++) {
		for (int c = 0; c < HMAP_DIM; c++) {
			HMapCell cell = rf->grid[r][c];
			if (hmap_cell_isempty(cell)) continue;

			HandType ht     = hmap_fromcoords(r, c);
			int      n_live = handtype_combos(ht, dead, buf);

			float sum = 0.0f;
			int   n   = 0;

			for (int sc = 0; sc < SUIT_CLASS_COUNT; sc++) {
				int count = hmap_cell_suit_total(cell, sc);
				if (count == 0) continue;

				/* Find the first live combo whose suit class matches sc.
				   Suit isomorphism guarantees all combos in this bucket
				   have identical equity against hero on this board. */
				uint64_t canonical = 0;
				for (int i = 0; i < n_live; i++) {
					uint64_t mask = combo_toBitmask(buf[i]);
					if ((int)classify_suit(board, mask) == sc) {
						canonical = mask;
						break;
					}
				}
				if (!canonical) continue; /* shouldn't happen: count > 0 but no match */

				/* Villain equity = equity of canonical combo against hero. */
				float eq = (float)equity_fast(board, canonical, hero);
				sum += eq * (float)count;
				n   += count;
			}

			if (n > 0) sf.grid[r][c] = sum / (float)n;
		}
	}

	return sf;
}
