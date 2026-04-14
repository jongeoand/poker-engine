#ifndef RENDER_H_
#define RENDER_H_

#include <stdio.h>
#include <stdint.h>

#include "card.h"
#include "combo.h"
#include "handtype.h"
#include "eval.h"
#include "draws.h"
#include "render_types.h"

typedef struct {
	FILE*      sink;        /* output destination; NULL → stdout */
	RenderMode mode;        /* what value each grid cell encodes */
	SymSet     symset;      /* ASCII or Unicode character set */
	CellWidth  width;       /* column width for grid rendering */
	int        term_width;  /* 0 = no width constraint */
} Renderer;

/* Lifecycle */
Renderer render_default(void);
void     render_set_sink(Renderer* r, FILE* f);
FILE*    render_get_sink(const Renderer* r);

/* Structural helpers */
void render_line(Renderer* r, const char* text);
void render_divider(Renderer* r, char ch, int width);
void render_heading(Renderer* r, const char* title);
void render_blank(Renderer* r);

/* Atom printers */
void render_card(Renderer* r, Card card);
void render_combo(Renderer* r, Combo combo);
void render_handtype(Renderer* r, HandType ht);
void render_hand_rank(Renderer* r, HandRank rank);
void render_equity(Renderer* r, double eq);
void render_board(Renderer* r, uint64_t board);
void render_draw_info(Renderer* r, const DrawInfo* d);
void render_binary(Renderer* r, uint64_t bitstring);

#endif
