#include <stdio.h>

#include "views.h"
#include "panel.h"
#include "analysis/celldata.h"

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

TextPanel* views_rangefield(Renderer* rend, const RangeField* f, const ScalarField* sf) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	char row[256];
	for (int ri = 0; ri < HMAP_DIM; ri++) {
		int len = 0;
		for (int col = 0; col < HMAP_DIM; col++) {
			if (rend->width != CELL_4) {
				/* CELL_1 and CELL_2: fully delegated to symbol_cell. */
				CellSample cs = {
				.cell   = f->grid[ri][col],
				.equity = (sf != NULL) ? sf->grid[ri][col] : -1.0f,
			};
				char sym_buf[16];
				symbol_cell(&cs, rend, sym_buf, sizeof(sym_buf));
				len += snprintf(row + len, sizeof(row) - (size_t)len, "%s", sym_buf);
			} else {
				/* CELL_4: label-based layout — hand type name + state glyph.
				   symbol_cell is not used here because the first three columns
				   are consumed by the rank label (e.g. "AKs", " QQ", "T9o"). */
				CellData d = cell_analyze(f->grid[ri][col]);
				if (d.empty) {
					len += snprintf(row + len, sizeof(row) - (size_t)len, "  .  ");
				} else {
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
					len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ",
					    symbol_state(rend->symset, d.dominant_state));
				}
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
	case RENDER_SUIT:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend:");
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=none",       symbol_suit(rend->symset, SUIT_NONE));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=backdoor",   symbol_suit(rend->symset, SUIT_BACKDOOR));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=flush-draw", symbol_suit(rend->symset, SUIT_FLUSH_DRAW));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=flush-made", symbol_suit(rend->symset, SUIT_FLUSH_MADE));
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "  %s=empty",      symbol_empty(rend->symset));
		panel_add_line(p, buf);
		break;
	case RENDER_FLUSH:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend (flush fraction, low→high):  ");
		for (int i = 0; i <= 8; i++)
			len += snprintf(buf + len, sizeof(buf) - (size_t)len, "%s ", symbol_ramp(rend->symset, i / 8.0));
		panel_add_line(p, buf);
		break;
	case RENDER_EQUITY:
		len += snprintf(buf + len, sizeof(buf) - (size_t)len, "Legend (villain equity, low→high):  ");
		for (int i = 0; i <= 8; i++)
			len += snprintf(buf + len, sizeof(buf) - (size_t)len, "%s ", symbol_ramp(rend->symset, i / 8.0));
		panel_add_line(p, buf);
		break;
	case RENDER_JOINT:
		/* Two lines: one per glyph family used in CELL_2 joint encoding. */
		len += snprintf(buf + len, sizeof(buf) - (size_t)len,
		    "Legend:  %s=ahead  %s=chop  %s=live  %s=dead  %s=empty",
		    symbol_state(rend->symset, COMBO_AHEAD),
		    symbol_state(rend->symset, COMBO_CHOP),
		    symbol_state(rend->symset, COMBO_BEHIND_LIVE),
		    symbol_state(rend->symset, COMBO_BEHIND_DEAD),
		    symbol_empty(rend->symset));
		panel_add_line(p, buf);
		len = 0;
		len += snprintf(buf + len, sizeof(buf) - (size_t)len,
		    "         %s=no-flush  %s=backdoor  %s=flush-draw  %s=flush-made",
		    symbol_suit(rend->symset, SUIT_NONE),
		    symbol_suit(rend->symset, SUIT_BACKDOOR),
		    symbol_suit(rend->symset, SUIT_FLUSH_DRAW),
		    symbol_suit(rend->symset, SUIT_FLUSH_MADE));
		panel_add_line(p, buf);
		break;
	case RENDER_ENTROPY:
	case RENDER_VOLATILITY:
		/* Not yet implemented. */
		break;
	}
	return p;
}

TextPanel* views_scalarfield(Renderer* r, const ScalarField* sf) {
	TextPanel* p = panel_create();
	if (!p) return NULL;
	char row[256];
	for (int ri = 0; ri < HMAP_DIM; ri++) {
		int len = 0;
		for (int col = 0; col < HMAP_DIM; col++) {
			float v = sf->grid[ri][col];
			if (v < 0.0f)
				len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_empty(r->symset));
			else
				len += snprintf(row + len, sizeof(row) - (size_t)len, "%s ", symbol_ramp(r->symset, (double)v));
		}
		panel_add_line(p, row);
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
