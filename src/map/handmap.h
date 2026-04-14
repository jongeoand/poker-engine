#ifndef HANDMAP_H_
#define HANDMAP_H_

#include "analysis/combostate.h"
#include "range/htrange.h"

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

#endif
