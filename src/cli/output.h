#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <stdio.h>
#include "render.h"
#include "views.h"
#include "htrange.h"
#include "map/handmap.h"

// Legacy config struct — kept for backward compatibility with existing test code.
// New code should use Renderer directly.
typedef struct {
	RenderMode mode;
	SymSet     symset;
	CellWidth  width;
} RenderConfig;

// Sink management — global sink used by all output_* wrapper functions.
void  output_set_sink(FILE* f);
FILE* output_get_sink(void);

// Atom printers (backward-compat wrappers over render_*)
void output_binary(uint64_t bitstring);
void output_card(Card card);
void output_combo(Combo combo);
void output_handtype(HandType ht);
void output_hand_rank(HandRank r);
void output_draw_info(const DrawInfo* d);
void output_board(uint64_t board);
void output_range(const Range* r);
void output_equity(double eq);

// HandTypeRange output (backward-compat wrappers over views_*)
void output_htr(const HandTypeRange* h);
void output_htr_board_profile(const HtrBoardProfile* p);
void output_htr_equity_split(const HtrEquitySplit* s);

// Topology rendering (backward-compat wrappers over views_*)
void output_rangefield(const RangeField* f, RenderConfig cfg);
void output_statefield(const StateField* f, RenderConfig cfg);

#endif
