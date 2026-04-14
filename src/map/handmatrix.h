#ifndef HANDMATRIX_H_
#define HANDMATRIX_H_

#include <stdint.h>
#include <assert.h>
#include "core/handtype.h"
// Grid layout — axis mapping: 12 - rank
//   ACE (rank 12) → axis 0    TWO (rank 0) → axis 12
//
//         col:   0     1     2     3   ·  ·  ·   11    12
//                A     K     Q     J              3     2
//               ┌─────┬─────┬─────┬─────┬─ · ─┬─────┬─────┐
//   row 0   A   │ AA  │ AKs │ AQs │ AJs │     │ A3s │ A2s │
//               ├─────┼─────┼─────┼─────┼─ · ─┼─────┼─────┤
//   row 1   K   │ AKo │ KK  │ KQs │ KJs │     │ K3s │ K2s │
//               ├─────┼─────┼─────┼─────┼─ · ─┼─────┼─────┤
//   row 2   Q   │ AQo │ KQo │ QQ  │ QJs │     │ Q3s │ Q2s │
//               ├─────┼─────┼─────┼─────┼─ · ─┼─────┼─────┤
//   row 3   J   │ AJo │ KJo │ QJo │ JJ  │     │ J3s │ J2s │
//               ├─────┼─────┼─────┼─────┼─ · ─┼─────┼─────┤
//    · · ·      │                                           │
//               ├─────┼─────┼─────┼─────┼─ · ─┼─────┼─────┤
//   row 12  2   │ A2o │ K2o │ Q2o │ J2o │     │ 32o │ 22  │
//               └─────┴─────┴─────┴─────┴─ · ─┴─────┴─────┘
//
//   Diagonal    (row == col): pairs
//   Above diag  (row <  col): suited   — high card → row, low card → col
//   Below diag  (row >  col): offsuit  — low  card → row, high card → col

#define HMAP_DIM 13

#define HandMatrix(T) struct { T grid[HMAP_DIM][HMAP_DIM]; }

// Axis ↔ rank: ACE (rank 12) → axis 0, TWO (rank 0) → axis 12.
// Higher rank = lower axis, so the field reads ace-first top-left.
static inline int rank_toaxis(int r)    { return 12 - r; }
static inline int axis_torank(int axis) { return 12 - axis; }

// hmap_tocoords: place ht in the standard hand matrix.
//   pair    → diagonal        (row == col == hi_axis)
//   suited  → above diagonal  (row = hi_axis, col = lo_axis  →  row < col)
//   offsuit → below diagonal  (row = lo_axis, col = hi_axis  →  row > col)
static void hmap_tocoords(HandType ht, int* row, int* col) {
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
static inline HandType hmap_fromcoords(int row, int col) {
	assert(row >= 0 && row < HMAP_DIM);
	assert(col >= 0 && col < HMAP_DIM);

	uint8_t r1 = (uint8_t)axis_torank(row);
	uint8_t r2 = (uint8_t)axis_torank(col);

	if (row == col) return make_pair(r1);
	if (row <  col) return make_suited(r1, r2);   // r1 = high, r2 = low
	return make_offsuit(r2, r1);                  // r2 = high, r1 = low
}

#endif
