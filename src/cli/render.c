#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "render.h"

Renderer render_default(void) {
	Renderer r;
	r.sink       = stdout;
	r.mode       = RENDER_STATE;
	r.symset     = SYMSET_ASCII;
	r.width      = CELL_1;
	r.term_width = 0;
	return r;
}

void  render_set_sink(Renderer* r, FILE* f) { r->sink = f ? f : stdout; }
FILE* render_get_sink(const Renderer* r)    { return r->sink ? r->sink : stdout; }

void render_line(Renderer* r, const char* text) {
	fprintf(render_get_sink(r), "%s\n", text);
}

void render_divider(Renderer* r, char ch, int width) {
	FILE* out = render_get_sink(r);
	for (int i = 0; i < width; i++) fputc(ch, out);
	fputc('\n', out);
}

void render_heading(Renderer* r, const char* title) {
	FILE* out = render_get_sink(r);
	fprintf(out, "%s\n", title);
	int len = (int)strlen(title);
	for (int i = 0; i < len; i++) fputc('-', out);
	fputc('\n', out);
}

void render_blank(Renderer* r) {
	fputc('\n', render_get_sink(r));
}

void render_card(Renderer* r, Card card) {
	FILE* out = render_get_sink(r);
	if (card.rank <= NINE)
		fprintf(out, "%d", card.rank + 2);
	else
		fprintf(out, "%c", rank_to_char(card.rank));
	fprintf(out, "%s", get_suit_string(card.suit));
}

void render_combo(Renderer* r, Combo combo) {
	render_card(r, combo.a);
	render_card(r, combo.b);
}

void render_handtype(Renderer* r, HandType ht) {
	FILE* out = render_get_sink(r);
	uint8_t high = ht.r_a >= ht.r_b ? ht.r_a : ht.r_b;
	uint8_t low  = ht.r_a <= ht.r_b ? ht.r_a : ht.r_b;
	fprintf(out, "%c%c", rank_to_char(high), rank_to_char(low));
	if (handtype_is_suited(ht))  fprintf(out, "s");
	if (handtype_is_offsuit(ht)) fprintf(out, "o");
}

void render_hand_rank(Renderer* r, HandRank rank) {
	fprintf(render_get_sink(r), "%s", hand_rank_str(rank));
}

void render_equity(Renderer* r, double eq) {
	fprintf(render_get_sink(r), "%.1f%%", eq * 100.0);
}

void render_board(Renderer* r, uint64_t board) {
	FILE* out = render_get_sink(r);
	bool first = true;
	for (int bit = 0; bit < 52; bit++) {
		if (!((board >> bit) & 1)) continue;
		if (!first) fprintf(out, " ");
		render_card(r, make_card(bit));
		first = false;
	}
}

void render_draw_info(Renderer* r, const DrawInfo* d) {
	FILE* out = render_get_sink(r);
	if (!d->flags) { fprintf(out, "no draws"); return; }
	if (d->flags & DRAW_FLUSH_DRAW)        fprintf(out, "FD(%d outs) ",  d->flush_outs);
	if (d->flags & DRAW_OESD)              fprintf(out, "OESD(%d outs) ", d->straight_outs);
	if (d->flags & DRAW_GUTSHOT)           fprintf(out, "GS(%d outs) ",  d->straight_outs);
	if (d->flags & DRAW_COMBO_DRAW)        fprintf(out, "COMBO ");
	if (d->flags & DRAW_BACKDOOR_FLUSH)    fprintf(out, "BDF ");
	if (d->flags & DRAW_BACKDOOR_STRAIGHT) fprintf(out, "BDS ");
}

void render_binary(Renderer* r, uint64_t bitstring) {
	FILE* out = render_get_sink(r);
	for (int s = 0; s < 4; s++) {
		for (int rank = 0; rank < 13; rank++)
			fprintf(out, "%d", (int)((bitstring >> (s * 13 + rank)) & 1));
		fputc('\n', out);
	}
}
