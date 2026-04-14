#ifndef HANDMAP_H_
#define HANDMAP_H_

#include "analysis/combostate.h"
#include "core/handtype.h"
#include "range/htrange.h"

#define HMAP_DIM 13

//
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

#define HandMatrix(T) struct { T grid[HMAP_DIM][HMAP_DIM]; }


// Combo state distribution for all combos belonging to one hand type.
// eg. for AKs: counts across {AhKh, AcKc, AdKd, AsKs} and their total.
typedef struct {
	int statecounts[COMBO_STATE_COUNT];
	int combo_total;
} HMapCell;

void hmap_cell_clear(HMapCell* p);
bool hmap_cell_isempty(const HMapCell* p);

// True when more than one ComboState bucket is non-zero.
bool hmap_cell_ismixed(const HMapCell* p);

// Increment the bucket for `state` and bump combo_total by 1.
void hmap_cell_add(HMapCell* p, ComboState state);
void hmap_cell_merge(HMapCell* dst, const HMapCell* src);


// RangeField 
typedef HandMatrix(HmapCell) Rangefield;

RangeField hmap_build(const HandTypeRange* htr, uint64_t dead, uint64_t board, uint64_t hero);

// Zero every HMapCell in the grid.
void hmap_clear(RangeField* f);

// Sum of combo_total across all 169 cells.
int hmap_total(const RangeField* f);

// Sum of the `s` bucket across all 169 cells.
int hmap_count(const RangeField* f, ComboState s);


// StateField
typedef HandMatrix(ComboState) StateField;

// Set every cell in the grid to `fill`.
void hmap_state_fill(StateField* f, ComboState fill);

// Project RangeField → StateField by selecting the dominant ComboState per cell.
// Empty cells (combo_total == 0) are assigned COMBO_BEHIND_DEAD as a sentinel.
// Returns out for chaining.
StateField* hmap_project_state(const RangeField* rf, StateField* out);

// Coordinate mapping: HandType ↔ (row, col) in the hand matrix above.
void     hmap_tocoords(HandType ht, int* row, int* col);
HandType hmap_fromcoords(int row, int col);

#endif
