#ifndef HANDMAP_H_
#define HANDMAP_H_

#include <stdint.h>
#include <stdbool.h>

#include "map/handmatrix.h"
#include "analysis/combostate.h"
#include "range/htrange.h"

// ------------------------------------------------------------------
// HMapCell — packed (ComboState × SuitClass) count matrix.
//
// A single uint64_t encodes a 4×4 grid of 4-bit nibble counts,
// one per (ComboState, SuitClass) pair:
//
//   bits [s*16 + u*4 + 3 : s*16 + u*4]  =  count for (state s, suit class u)
//
// Max value per nibble: 12 (max combos per hand type for offsuit hands).
// 12 < 16 = 2^4, so no nibble can overflow.
//
// Constraint: COMBO_STATE_COUNT == 4 and SUIT_CLASS_COUNT == 4
// (4 states × 4 suit classes × 4 bits = 64 bits exactly).
// ------------------------------------------------------------------
typedef uint64_t HMapCell;

#define HMAP_CELL_SHIFT(state, suit) ((state) * 16 + (suit) * 4)

// Count for a specific (state, suit) pair.
static inline int hmap_cell_get(HMapCell cell, int state, int suit) {
	return (int)((cell >> HMAP_CELL_SHIFT(state, suit)) & 0xFULL);
}

// Increment the count for (state, suit) by 1.
static inline void hmap_cell_add(HMapCell* p, int state, int suit) {
	*p += (1ULL << HMAP_CELL_SHIFT(state, suit));
}

// True when no combos have been recorded (all nibbles zero).
static inline bool hmap_cell_isempty(HMapCell cell) {
	return cell == 0;
}

// Total combos in a given ComboState (sum across all SuitClasses).
static inline int hmap_cell_state_total(HMapCell cell, int state) {
	int n = 0;
	for (int u = 0; u < SUIT_CLASS_COUNT; u++)
		n += hmap_cell_get(cell, state, u);
	return n;
}

// Total combos in a given SuitClass (sum across all ComboStates).
static inline int hmap_cell_suit_total(HMapCell cell, int suit) {
	int n = 0;
	for (int s = 0; s < COMBO_STATE_COUNT; s++)
		n += hmap_cell_get(cell, s, suit);
	return n;
}

// Total combos in the cell (sum of all 16 nibbles).
static inline int hmap_cell_total(HMapCell cell) {
	int n = 0;
	for (int s = 0; s < COMBO_STATE_COUNT; s++)
		n += hmap_cell_state_total(cell, s);
	return n;
}

// True when more than one ComboState bucket is non-zero.
static inline bool hmap_cell_ismixed(HMapCell cell) {
	int seen = 0;
	for (int s = 0; s < COMBO_STATE_COUNT; s++)
		if (hmap_cell_state_total(cell, s) > 0 && ++seen > 1) return true;
	return false;
}

// Merge src into dst nibble-by-nibble (direct addition would carry across nibbles).
void hmap_cell_merge(HMapCell* dst, HMapCell src);

// ------------------------------------------------------------------
// RangeField — 13×13 grid of HMapCell, one per hand type.
// ------------------------------------------------------------------
typedef HandMatrix(HMapCell) RangeField;

// Zero every cell.
void hmap_clear(RangeField* f);

// Build a RangeField from a support and game state.
//   dead  = board | hero bitmask  (blocks combos from the stream)
//   board = community card bitmask (ComboState + SuitClass classification)
//   hero  = hero hole card bitmask
RangeField hmap_build(const HandTypeRange* htr, uint64_t dead, uint64_t board, uint64_t hero);

// Sum of hmap_cell_total across all 169 cells.
int hmap_total(const RangeField* f);

// Sum of hmap_cell_state_total(cell, s) across all 169 cells.
int hmap_count(const RangeField* f, ComboState s);

// ------------------------------------------------------------------
// ScalarField — 13×13 grid of float, one value per hand type.
//
// Each cell holds the mean equity of that hand type against a specific
// hero holding, derived from a pre-built RangeField.
//
// Suit-isomorphism optimisation: combos that are equivalent under suit
// relabelling on the given board share the same equity, so only one
// equity call is made per (hand_type, SuitClass) bucket.  The bucket
// counts come directly from the RangeField, so at most
// 169 × SUIT_CLASS_COUNT equity evaluations are needed (vs up to 1326
// if every combo were evaluated independently).
//
// Sentinel: cells with no live combos are set to -1.0f.
// ------------------------------------------------------------------
typedef HandMatrix(float) ScalarField;

// Build a ScalarField from an existing RangeField.
//   rf    = pre-built villain range (from hmap_build)
//   dead  = board | hero bitmask  (same mask used for hmap_build)
//   board = community card bitmask
//   hero  = hero hole card bitmask
ScalarField scalar_build(const RangeField* rf, uint64_t dead,
                         uint64_t board, uint64_t hero);
#endif
