#ifndef CELLDATA_H_
#define CELLDATA_H_

#include <stdbool.h>

#include "map/handmap.h"

typedef struct {
    bool empty;
    ComboState dominant_state;
    SuitClass dominant_suit;
    SuitClass dominant_suit_in_state;
    double dom_frac;
    double flush_frac;
    double draw_frac;
} CellData;

CellData cell_analyze(HMapCell cell);

#endif
