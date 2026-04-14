#include <stdio.h>

#include "views.h"
#include "panel.h"

// ---- Helpers ----

// Select the dominant ComboState for a cell and compute fractions.
// Returns 0 if the cell is empty.
static int cell_analysis(HMapCell cell,
                         ComboState* dominant,
                         double* dom_frac,
                         double* draw_frac)
{
	int total = hmap_cell_total(cell);
	if (total == 0) return 0;

	*dominant = COMBO_AHEAD;
	for (int s = 1; s < COMBO_STATE_COUNT; s++)
		if (hmap_cell_state_total(cell, s) > hmap_cell_state_total(cell, (int)*dominant))
			*dominant = (ComboState)s;

	*dom_frac  = (double)hmap_cell_state_total(cell, (int)*dominant) / total;
	*draw_frac = (double)hmap_cell_state_total(cell, COMBO_BEHIND_LIVE) / total;
	return 1;
}

// ---- Views ----

TextPanel* views_htr_grid(const HandTypeRange* h) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	char row[64];
	for (int r = 12; r >= 0; r--) {
		int len = 0;
		for (int c = 12; c >= 0; c--) {
			HandType ht;
			if (r == c)     ht = make_pair((uint8_t)r);
			else if (r > c) ht = make_suited((uint8_t)r, (uint8_t)c);
			else            ht = make_offsuit((uint8_t)c, (uint8_t)r);

			if (htr_contains(h, ht)) {
				char hi = rank_to_char((uint8_t)(r > c ? r : c));
				char lo = rank_to_char((uint8_t)(r < c ? r : c));
				if (r == c)
					len += snprintf(row + len, sizeof(row) - (size_t)len, " %c%c ", hi, lo);
				else if (r > c)
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%c%cs ", hi, lo);
				else
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%c%co ", hi, lo);
			} else {
				len += snprintf(row + len, sizeof(row) - (size_t)len, "  . ");
			}
		}
		panel_add_line(p, row);
	}
	return p;
}

TextPanel* views_rangefield(Renderer* rend, const RangeField* f) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	char row[256];
	for (int ri = 0; ri < HMAP_DIM; ri++) {
		int len = 0;
		for (int col = 0; col < HMAP_DIM; col++) {
			HMapCell cell = f->grid[ri][col];
			ComboState dominant;
			double dom_frac, draw_frac;
			int filled = cell_analysis(cell, &dominant, &dom_frac, &draw_frac);

			switch (rend->width) {
			case CELL_1:
				if (!filled) {
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_empty(rend->symset));
					break;
				}
				switch (rend->mode) {
				case RENDER_STATE:
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_state(rend->symset, dominant));
					break;
				case RENDER_PURITY:
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_ramp(rend->symset, dom_frac));
					break;
				case RENDER_DRAW:
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_ramp(rend->symset, draw_frac));
					break;
				}
				break;

			case CELL_2:
				if (!filled) {
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s  ", symbol_empty(rend->symset));
					break;
				}
				len += snprintf(row + len, sizeof(row) - (size_t)len, "%s%s ",
					symbol_state(rend->symset, dominant),
					symbol_ramp(rend->symset, dom_frac));
				break;

			case CELL_4:
				if (!filled) {
					len += snprintf(row + len, sizeof(row) - (size_t)len, "  .  ");
					break;
				}
				{
					uint8_t hi_rank = (uint8_t)(ri <= col ? 12 - ri : 12 - col);
					uint8_t lo_rank = (uint8_t)(ri <= col ? 12 - col : 12 - ri);
					char hi = rank_to_char(hi_rank);
					char lo = rank_to_char(lo_rank);
					if (ri == col)
						len += snprintf(row + len, sizeof(row) - (size_t)len, " %c%c", hi, lo);
					else if (ri < col)
						len += snprintf(row + len, sizeof(row) - (size_t)len, "%c%cs", hi, lo);
					else
						len += snprintf(row + len, sizeof(row) - (size_t)len, "%c%co", hi, lo);
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_state(rend->symset, dominant));
				}
				break;
			}
		}
		panel_add_line(p, row);
	}
	return p;
}

TextPanel* views_legend(Renderer* rend) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	char buf[256]; int len = 0;
	switch (rend->mode) {
	case RENDER_STATE:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend:");
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=ahead",       symbol_state(rend->symset, COMBO_AHEAD));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=chop",        symbol_state(rend->symset, COMBO_CHOP));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=behind-live", symbol_state(rend->symset, COMBO_BEHIND_LIVE));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=behind-dead", symbol_state(rend->symset, COMBO_BEHIND_DEAD));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=empty",       symbol_empty(rend->symset));
		panel_add_line(p, buf);
		break;
	case RENDER_PURITY:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend (dominant state purity, low→high):  ");
		for (int i = 0; i <= 8; i++)
			len += snprintf(buf + len, sizeof(buf) - (size_t)len, "%s ", symbol_ramp(rend->symset, i / 8.0));
		panel_add_line(p, buf);
		break;
	case RENDER_DRAW:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend (draw proportion, low→high):  ");
		for (int i = 0; i <= 8; i++)
			len += snprintf(buf + len, sizeof(buf) - (size_t)len, "%s ", symbol_ramp(rend->symset, i / 8.0));
		panel_add_line(p, buf);
		break;
	}
	return p;
}

TextPanel* views_state_summary(const ComboStateCounts* c) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	static const char* labels[4] = { "Ahead", "Chop", "Behind-live", "Behind-dead" };
	char buf[64];
	for (int i = 0; i < 4; i++) {
		double pct = c->total > 0 ? 100.0 * c->counts[i] / c->total : 0.0;
		snprintf(buf, sizeof(buf), "  %-12s  %4d  (%5.1f%%)", labels[i], c->counts[i], pct);
		panel_add_line(p, buf);
	}
	snprintf(buf, sizeof(buf), "  %-12s  %4d", "Total", c->total);
	panel_add_line(p, buf);
	return p;
}
