#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "output.h"

// Global sink for backward-compatible output_* wrappers.
static FILE* g_sink = NULL;

void  output_set_sink(FILE* f) { g_sink = f; }
FILE* output_get_sink(void)    { return g_sink ? g_sink : stdout; }

// Build a Renderer pointing at the global sink with a given config.
static Renderer make_renderer(RenderConfig cfg) {
	Renderer r;
	r.sink       = output_get_sink();
	r.mode       = cfg.mode;
	r.symset     = cfg.symset;
	r.width      = cfg.width;
	r.term_width = 0;
	return r;
}

// Build a default Renderer pointing at the global sink.
static Renderer sink_renderer(void) {
	Renderer r = render_default();
	r.sink = output_get_sink();
	return r;
}

// ---- Atom printers ----

void output_binary(uint64_t bitstring) { Renderer r = sink_renderer(); render_binary(&r, bitstring); }
void output_card(Card card)            { Renderer r = sink_renderer(); render_card(&r, card); }
void output_combo(Combo combo)         { Renderer r = sink_renderer(); render_combo(&r, combo); }
void output_handtype(HandType ht)      { Renderer r = sink_renderer(); render_handtype(&r, ht); }
void output_hand_rank(HandRank rank)   { Renderer r = sink_renderer(); render_hand_rank(&r, rank); }
void output_draw_info(const DrawInfo* d) { Renderer r = sink_renderer(); render_draw_info(&r, d); }
void output_board(uint64_t board)      { Renderer r = sink_renderer(); render_board(&r, board); }
void output_equity(double eq)          { Renderer r = sink_renderer(); render_equity(&r, eq); }

void output_range(const Range* range) {
	FILE* out = output_get_sink();
	bool first = true;
	for (int i = 0; i < COMBO_RANGE_WORDS; i++) {
		uint32_t word = range->bits[i];
		while (word) {
			int bit = __builtin_ctz(word);
			word &= word - 1;
			int idx = i * 32 + bit;
			if (idx >= COMBO_COUNT) break;
			if (!first) fprintf(out, " ");
			output_combo(combo_from_index(idx));
			first = false;
		}
	}
	fprintf(out, "\n");
}

// ---- HandTypeRange output ----

void output_htr(const HandTypeRange* h) {
	Renderer r = sink_renderer();
	views_htr_grid(&r, h);
}

void output_htr_board_profile(const HtrBoardProfile* p) {
	Renderer r = sink_renderer();
	views_board_profile(&r, p);
}

void output_htr_equity_split(const HtrEquitySplit* s) {
	FILE* out = output_get_sink();
	fprintf(out, "Ahead  (%d types):\n", htr_count(&s->ahead));
	output_htr(&s->ahead);
	fprintf(out, "Behind (%d types):\n", htr_count(&s->behind));
	output_htr(&s->behind);
	fprintf(out, "Chop   (%d types):\n", htr_count(&s->chopping));
	output_htr(&s->chopping);
}

// ---- Topology rendering ----

void output_rangefield(const RangeField* f, RenderConfig cfg) {
	Renderer r = make_renderer(cfg);
	views_rangefield(&r, f);
}

void output_statefield(const StateField* f, RenderConfig cfg) {
	Renderer r = make_renderer(cfg);
	views_statefield(&r, f);
}
