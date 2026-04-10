#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <stdio.h>
#include "htrange.h"
#include "map/handmap.h"

typedef enum { RENDER_STATE, RENDER_PURITY, RENDER_DRAW } RenderMode;
typedef enum { SYMSET_ASCII, SYMSET_UNICODE }             SymSet;
typedef enum { CELL_1, CELL_2, CELL_4 }                  CellWidth;

typedef struct {
	RenderMode mode;
	SymSet     symset;
	CellWidth  width;
} RenderConfig;

// Sink management — defaults to stdout.
// All output_* functions write to the current sink.
void  output_set_sink(FILE* f);
FILE* output_get_sink(void);

void output_binary(uint64_t bitstring);
void output_card(Card card);
void output_combo(Combo combo);
void output_handtype(HandType ht);
void output_hand_rank(HandRank r);
void output_draw_info(const DrawInfo* d);
void output_board(uint64_t board);
void output_range(const Range* r);
void output_equity(double eq);

// HandTypeRange output
void output_htr(const HandTypeRange* h);
void output_htr_board_profile(const HtrBoardProfile* p);
void output_htr_equity_split(const HtrEquitySplit* s);

// Topology rendering (RangeField / StateField)
void output_rangefield(const RangeField* f, RenderConfig cfg);
void output_statefield(const StateField* f, RenderConfig cfg);

#endif
