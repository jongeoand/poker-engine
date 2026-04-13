#include <stdio.h>

#include "views.h"

// ---- Helpers ----

// Select the dominant ComboState for a cell and compute fractions.
// Returns 0 if cell is empty (combo_total == 0).
static int cell_analysis(const HMapCell* cell,
                         ComboState* dominant,
                         double* dom_frac,
                         double* draw_frac)
{
	if (cell->combo_total == 0) return 0;

	*dominant = COMBO_AHEAD;
	for (int s = 1; s < COMBO_STATE_COUNT; s++)
		if (cell->statecounts[s] > cell->statecounts[*dominant])
			*dominant = (ComboState)s;

	*dom_frac  = (double)cell->statecounts[*dominant] / cell->combo_total;
	*draw_frac = (double)cell->statecounts[COMBO_BEHIND_LIVE] / cell->combo_total;
	return 1;
}

// ---- Views ----

void views_htr_grid(Renderer* rend, const HandTypeRange* h) {
	FILE* out = render_get_sink(rend);
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
		fputc('\n', out);
	}
	fputc('\n', out);
}

void views_rangefield(Renderer* rend, const RangeField* f) {
	FILE* out = render_get_sink(rend);
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++) {
			const HMapCell* cell = &f->grid[row][col];
			ComboState dominant;
			double dom_frac, draw_frac;
			int filled = cell_analysis(cell, &dominant, &dom_frac, &draw_frac);

			switch (rend->width) {
			case CELL_1:
				if (!filled) {
					fprintf(out, "%s ", symbol_empty(rend->symset));
					break;
				}
				switch (rend->mode) {
				case RENDER_STATE:
					fprintf(out, "%s ", symbol_state(rend->symset, dominant));
					break;
				case RENDER_PURITY:
					fprintf(out, "%s ", symbol_ramp(rend->symset, dom_frac));
					break;
				case RENDER_DRAW:
					fprintf(out, "%s ", symbol_ramp(rend->symset, draw_frac));
					break;
				}
				break;

			case CELL_2:
				if (!filled) {
					fprintf(out, "%s  ", symbol_empty(rend->symset));
					break;
				}
				fprintf(out, "%s%s ",
					symbol_state(rend->symset, dominant),
					symbol_ramp(rend->symset, dom_frac));
				break;

			case CELL_4:
				if (!filled) {
					fputs("  .  ", out);
					break;
				}
				{
					uint8_t hi_rank = (uint8_t)(row <= col ? 12 - row : 12 - col);
					uint8_t lo_rank = (uint8_t)(row <= col ? 12 - col : 12 - row);
					char hi = rank_to_char(hi_rank);
					char lo = rank_to_char(lo_rank);
					if (row == col)
						fprintf(out, " %c%c", hi, lo);
					else if (row < col)
						fprintf(out, "%c%cs", hi, lo);
					else
						fprintf(out, "%c%co", hi, lo);
					fprintf(out, "%s ", symbol_state(rend->symset, dominant));
				}
				break;
			}
		}
		fputc('\n', out);
	}
}

void views_statefield(Renderer* rend, const StateField* f) {
	FILE* out = render_get_sink(rend);
	for (int row = 0; row < HMAP_DIM; row++) {
		for (int col = 0; col < HMAP_DIM; col++)
			fprintf(out, "%s ", symbol_state(rend->symset, f->grid[row][col]));
		fputc('\n', out);
	}
}

void views_legend(Renderer* rend) {
	FILE* out = render_get_sink(rend);
	switch (rend->mode) {
	case RENDER_STATE:
		fprintf(out, "Legend:");
		fprintf(out, "  %s=ahead", symbol_state(rend->symset, COMBO_AHEAD));
		fprintf(out, "  %s=chop", symbol_state(rend->symset, COMBO_CHOP));
		fprintf(out, "  %s=behind-live", symbol_state(rend->symset, COMBO_BEHIND_LIVE));
		fprintf(out, "  %s=behind-dead", symbol_state(rend->symset, COMBO_BEHIND_DEAD));
		fprintf(out, "  %s=empty\n", symbol_empty(rend->symset));
		break;
	case RENDER_PURITY:
		fprintf(out, "Legend (dominant state purity, low→high):  ");
		for (int i = 0; i <= 8; i++)
			fprintf(out, "%s ", symbol_ramp(rend->symset, i / 8.0));
		fputc('\n', out);
		break;
	case RENDER_DRAW:
		fprintf(out, "Legend (draw proportion, low→high):  ");
		for (int i = 0; i <= 8; i++)
			fprintf(out, "%s ", symbol_ramp(rend->symset, i / 8.0));
		fputc('\n', out);
		break;
	}
}

void views_state_summary(Renderer* rend, const ComboStateCounts* c) {
	FILE* out = render_get_sink(rend);
	static const char* labels[4] = { "Ahead", "Chop", "Behind-live", "Behind-dead" };
	for (int i = 0; i < 4; i++) {
		double pct = c->total > 0 ? 100.0 * c->counts[i] / c->total : 0.0;
		fprintf(out, "  %-12s  %4d  (%5.1f%%)\n", labels[i], c->counts[i], pct);
	}
	fprintf(out, "  %-12s  %4d\n", "Total", c->total);
}
