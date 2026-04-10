#include <stdio.h>
#include <stdint.h>

#include "output.h"

static FILE* g_sink = NULL;

void output_set_sink(FILE* f) { g_sink = f; }
FILE* output_get_sink(void)   { return g_sink ? g_sink : stdout; }

// ---- Print functions ----

void output_binary(uint64_t bitstring) {
	FILE* out = output_get_sink();
	for (int s = 0; s < 4; s++) {
		for (int r = 0; r < 13; r++) {
			fprintf(out, "%d", (int)((bitstring >> (s * 13 + r)) & 1));
		}
		fprintf(out, "\n");
	}
}

void output_card(Card card) {
	FILE* out = output_get_sink();
	if (card.rank <= NINE)
		fprintf(out, "%d", card.rank + 2);
	else
		fprintf(out, "%c", rank_to_char(card.rank));
	fprintf(out, "%s", get_suit_string(card.suit));
}

void output_combo(Combo combo) {
	output_card(combo.a);
	output_card(combo.b);
}

void output_handtype(HandType ht) {
	FILE* out = output_get_sink();
	uint8_t high = ht.r_a >= ht.r_b ? ht.r_a : ht.r_b;
	uint8_t low  = ht.r_a <= ht.r_b ? ht.r_a : ht.r_b;
	fprintf(out, "%c%c", rank_to_char(high), rank_to_char(low));
	if (handtype_is_suited(ht))  fprintf(out, "s");
	if (handtype_is_offsuit(ht)) fprintf(out, "o");
}

void output_hand_rank(HandRank r) {
	fprintf(output_get_sink(), "%s", hand_rank_str(r));
}

void output_draw_info(const DrawInfo* d) {
	FILE* out = output_get_sink();
	if (!d->flags) {
		fprintf(out, "no draws");
		return;
	}
	if (d->flags & DRAW_FLUSH_DRAW)        fprintf(out, "FD(%d outs) ",  d->flush_outs);
	if (d->flags & DRAW_OESD)              fprintf(out, "OESD(%d outs) ", d->straight_outs);
	if (d->flags & DRAW_GUTSHOT)           fprintf(out, "GS(%d outs) ",  d->straight_outs);
	if (d->flags & DRAW_COMBO_DRAW)        fprintf(out, "COMBO ");
	if (d->flags & DRAW_BACKDOOR_FLUSH)    fprintf(out, "BDF ");
	if (d->flags & DRAW_BACKDOOR_STRAIGHT) fprintf(out, "BDS ");
}

void output_board(uint64_t board) {
	FILE* out = output_get_sink();
	bool first = true;
	for (int bit = 0; bit < 52; bit++) {
		if (!((board >> bit) & 1)) continue;
		if (!first) fprintf(out, " ");
		output_card(make_card(bit));
		first = false;
	}
}

void output_range(const Range* r) {
	FILE* out = output_get_sink();
	bool first = true;
	for (int i = 0; i < COMBO_RANGE_WORDS; i++) {
		uint32_t word = r->bits[i];
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

void output_equity(double eq) {
	fprintf(output_get_sink(), "%.1f%%", eq * 100.0);
}

// ---- HandTypeRange output ----

// 13×13 grid: rows/cols run from ACE(12) down to TWO(0).
// Upper-left diagonal = pairs, upper triangle = suited, lower triangle = offsuit.
// Each cell is 4 chars wide. In-range: " AA", "AKs", "AKo" + space. Out: "  . ".
void output_htr(const HandTypeRange* h) {
	FILE* out = output_get_sink();
	for (int r = 12; r >= 0; r--) {
		for (int c = 12; c >= 0; c--) {
			HandType ht;
			if (r == c)     ht = make_pair((uint8_t)r);
			else if (r > c) ht = make_suited((uint8_t)r, (uint8_t)c);
			else            ht = make_offsuit((uint8_t)c, (uint8_t)r);

			if (htr_contains(h, ht)) {
				char hi = rank_to_char((uint8_t)(r > c ? r : c));
				char lo = rank_to_char((uint8_t)(r < c ? r : c));
				if (r == c)
					fprintf(out, " %c%c ", hi, lo);
				else if (r > c)
					fprintf(out, "%c%cs ", hi, lo);
				else
					fprintf(out, "%c%co ", hi, lo);
			} else {
				fprintf(out, "  . ");
			}
		}
		fprintf(out, "\n");
	}
	fprintf(out, "\n");
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

// ---- RangeField / StateField rendering ----

static const char* state_unicode_sym(ComboState s) {
	switch (s) {
		case COMBO_AHEAD:       return "\xe2\x96\x88"; // █
		case COMBO_CHOP:        return "\xe2\x94\x80"; // ─
		case COMBO_BEHIND_LIVE: return "\xe2\x96\x92"; // ▒
		case COMBO_BEHIND_DEAD: return "\xe2\x96\x91"; // ░
		default:                return "\xc2\xb7";     // ·
	}
}

static const char* ASCII_RAMP[8]   = { ".", ":", "-", "=", "+", "*", "#", "@" };
static const char* UNICODE_RAMP[9] = { "\xc2\xb7","▁","▂","▃","▄","▅","▆","▇","█" };

static int ramp_index(double frac, int levels) {
	int i = (int)(frac * levels);
	return i >= levels ? levels - 1 : i < 0 ? 0 : i;
}

void output_rangefield(const RangeField* f, RenderConfig cfg) {
	FILE* sink = output_get_sink();
	for (int r = 0; r < HMAP_DIM; r++) {
		for (int c = 0; c < HMAP_DIM; c++) {
			const HMapCell* cell = &f->grid[r][c];
			int empty = (cell->combo_total == 0);

			ComboState dominant = COMBO_BEHIND_DEAD;
			double dom_frac = 0.0, draw_frac = 0.0;
			if (!empty) {
				dominant = COMBO_AHEAD;
				for (int s = 1; s < COMBO_STATE_COUNT; s++)
					if (cell->statecounts[s] > cell->statecounts[dominant])
						dominant = (ComboState)s;
				dom_frac  = (double)cell->statecounts[dominant] / cell->combo_total;
				draw_frac = (double)cell->statecounts[COMBO_BEHIND_LIVE] / cell->combo_total;
			}

			switch (cfg.width) {
			case CELL_1:
				if (empty) {
					fputs(cfg.symset == SYMSET_ASCII ? ". " : "\xc2\xb7 ", sink);
					break;
				}
				switch (cfg.mode) {
				case RENDER_STATE:
					if (cfg.symset == SYMSET_ASCII)
						fprintf(sink, "%c ", combostate_symbol(dominant));
					else
						fprintf(sink, "%s ", state_unicode_sym(dominant));
					break;
				case RENDER_PURITY:
					fprintf(sink, "%s ",
						cfg.symset == SYMSET_ASCII
							? ASCII_RAMP[ramp_index(dom_frac, 8)]
							: UNICODE_RAMP[ramp_index(dom_frac, 9)]);
					break;
				case RENDER_DRAW:
					fprintf(sink, "%s ",
						cfg.symset == SYMSET_ASCII
							? ASCII_RAMP[ramp_index(draw_frac, 8)]
							: UNICODE_RAMP[ramp_index(draw_frac, 9)]);
					break;
				}
				break;

			case CELL_2:
				if (empty) {
					fputs(cfg.symset == SYMSET_ASCII ? ".  " : "\xc2\xb7  ", sink);
					break;
				}
				if (cfg.symset == SYMSET_ASCII)
					fprintf(sink, "%c%s ", combostate_symbol(dominant),
						ASCII_RAMP[ramp_index(dom_frac, 8)]);
				else
					fprintf(sink, "%s%s ", state_unicode_sym(dominant),
						UNICODE_RAMP[ramp_index(dom_frac, 9)]);
				break;

			case CELL_4:
				if (empty) {
					fputs("  .  ", sink);
					break;
				}
				{
					uint8_t hi_rank = (uint8_t)(r <= c ? 12 - r : 12 - c);
					uint8_t lo_rank = (uint8_t)(r <= c ? 12 - c : 12 - r);
					char hi = rank_to_char(hi_rank);
					char lo = rank_to_char(lo_rank);
					if (r == c)
						fprintf(sink, " %c%c", hi, lo);
					else if (r < c)
						fprintf(sink, "%c%cs", hi, lo);
					else
						fprintf(sink, "%c%co", hi, lo);
					if (cfg.symset == SYMSET_ASCII)
						fprintf(sink, "%c ", combostate_symbol(dominant));
					else
						fprintf(sink, "%s ", state_unicode_sym(dominant));
				}
				break;
			}
		}
		fputc('\n', sink);
	}
}

void output_statefield(const StateField* f, RenderConfig cfg) {
	FILE* sink = output_get_sink();
	for (int r = 0; r < HMAP_DIM; r++) {
		for (int c = 0; c < HMAP_DIM; c++) {
			ComboState s = f->grid[r][c];
			if (cfg.symset == SYMSET_ASCII)
				fprintf(sink, "%c ", combostate_symbol(s));
			else
				fprintf(sink, "%s ", state_unicode_sym(s));
		}
		fputc('\n', sink);
	}
}

void output_htr_board_profile(const HtrBoardProfile* p) {
	FILE* out = output_get_sink();
	fprintf(out, "Board profile (%d live types):\n", p->total);
	for (int r = STRAIGHTFLUSH; r >= PAIRED; r--) {
		if (!p->made[r]) continue;
		fprintf(out, "  %-16s  %3d  (%5.1f%%)\n",
			hand_rank_str((HandRank)r), p->made[r],
			p->total > 0 ? 100.0 * p->made[r] / p->total : 0.0);
	}
	fprintf(out, "  %-16s  %3d  (%5.1f%%)\n",
		"Draw", p->draw,
		p->total > 0 ? 100.0 * p->draw / p->total : 0.0);
	fprintf(out, "  %-16s  %3d  (%5.1f%%)\n",
		"Missed (air)", p->missed,
		p->total > 0 ? 100.0 * p->missed / p->total : 0.0);
}
