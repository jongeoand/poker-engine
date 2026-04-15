#include "symbols.h"
#include "analysis/celldata.h"
#include <stddef.h>
#include <stdio.h>

static const char* STATE_ASCII[4] = { "A", "C", "L", "D" };

static const char* STATE_UNICODE[4] = {
	"\xe2\x96\x88",  /* █  COMBO_AHEAD       */
	"\xe2\x94\x80",  /* ─  COMBO_CHOP        */
	"\xe2\x96\x92",  /* ▒  COMBO_BEHIND_LIVE */
	"\xe2\x96\x91",  /* ░  COMBO_BEHIND_DEAD */
};

static const char* RAMP_ASCII[8] = { ".", ":", "-", "=", "+", "*", "#", "@" };

static const char* RAMP_UNICODE[9] = {
	"\xc2\xb7",      /* ·  */
	"\xe2\x96\x81",  /* ▁  */
	"\xe2\x96\x82",  /* ▂  */
	"\xe2\x96\x83",  /* ▃  */
	"\xe2\x96\x84",  /* ▄  */
	"\xe2\x96\x85",  /* ▅  */
	"\xe2\x96\x86",  /* ▆  */
	"\xe2\x96\x87",  /* ▇  */
	"\xe2\x96\x88",  /* █  */
};

static int ramp_index(double frac, int levels) {
	int i = (int)(frac * levels);
	return i >= levels ? levels - 1 : i < 0 ? 0 : i;
}

const char* symbol_state(SymSet s, ComboState state) {
	int idx = (int)state;
	if (idx < 0 || idx >= 4) return "?";
	return s == SYMSET_UNICODE ? STATE_UNICODE[idx] : STATE_ASCII[idx];
}

const char* symbol_ramp(SymSet s, double frac) {
	if (s == SYMSET_UNICODE)
		return RAMP_UNICODE[ramp_index(frac, 9)];
	return RAMP_ASCII[ramp_index(frac, 8)];
}

/* Circle-fill progression — perceptually distinct from block-family state symbols.
   SUIT_NONE=○  SUIT_BACKDOOR=◑  SUIT_FLUSH_DRAW=◕  SUIT_FLUSH_MADE=● */
static const char* SUIT_ASCII[4]   = { "n", "b", "f", "F" };
static const char* SUIT_UNICODE[4] = {
	"\xe2\x97\x8b",  /* ○  SUIT_NONE       (U+25CB) */
	"\xe2\x97\x91",  /* ◑  SUIT_BACKDOOR   (U+25D1) */
	"\xe2\x97\x95",  /* ◕  SUIT_FLUSH_DRAW (U+25D5) */
	"\xe2\x97\x8f",  /* ●  SUIT_FLUSH_MADE (U+25CF) */
};

const char* symbol_suit(SymSet s, SuitClass suit) {
	int idx = (int)suit;
	if (idx < 0 || idx >= 4) return "?";
	return s == SYMSET_UNICODE ? SUIT_UNICODE[idx] : SUIT_ASCII[idx];
}

const char* symbol_empty(SymSet s) {
	return s == SYMSET_UNICODE ? "\xc2\xb7" : ".";
}

void symbol_cell(const CellSample* s, const Renderer* r, char* buf, size_t bufsz) {
	/* Empty cell: consistent placeholder regardless of mode or width. */
	if (hmap_cell_isempty(s->cell)) {
		const char* e = symbol_empty(r->symset);
		if (r->width == CELL_1)
			snprintf(buf, bufsz, "%s ", e);
		else
			snprintf(buf, bufsz, "%s  ", e);   /* CELL_2 and CELL_4 fallback */
		return;
	}

	CellData d = cell_analyze(s->cell);

	if (r->width == CELL_1) {
		const char* sym;
		switch (r->mode) {
		case RENDER_STATE:
			sym = symbol_state(r->symset, d.dominant_state);
			break;
		case RENDER_PURITY:
			sym = symbol_ramp(r->symset, d.dom_frac);
			break;
		case RENDER_DRAW:
			sym = symbol_ramp(r->symset, d.draw_frac);
			break;
		case RENDER_SUIT:
			sym = symbol_suit(r->symset, d.dominant_suit);
			break;
		case RENDER_FLUSH:
			sym = symbol_ramp(r->symset, d.flush_frac);
			break;
		case RENDER_EQUITY:
			/* Use equity ramp when available; degrade to dom_frac otherwise. */
			sym = (s->equity >= 0.0f)
			    ? symbol_ramp(r->symset, (double)s->equity)
			    : symbol_ramp(r->symset, d.dom_frac);
			break;
		case RENDER_JOINT:
			/* Cannot represent both dimensions in one glyph — show state. */
			sym = symbol_state(r->symset, d.dominant_state);
			break;
		case RENDER_ENTROPY:
		case RENDER_VOLATILITY:
		default:
			/* Not yet implemented — degrade to purity ramp. */
			sym = symbol_ramp(r->symset, d.dom_frac);
			break;
		}
		snprintf(buf, bufsz, "%s ", sym);
		return;
	}

	/* CELL_2 (and CELL_4 fallback): two meaningful glyphs + one space.
	   Convention: char[0] = primary signal, char[1] = orthogonal context.
	   RENDER_JOINT is the canonical two-axis mode; single-axis modes pair
	   their primary symbol with the most informative available second axis. */
	const char* p1;
	const char* p2;
	switch (r->mode) {
	case RENDER_STATE:
		/* [state][purity_ramp] — dominant state with homogeneity modifier. */
		p1 = symbol_state(r->symset, d.dominant_state);
		p2 = symbol_ramp(r->symset, d.dom_frac);
		break;
	case RENDER_PURITY:
		/* [purity_ramp][state] — purity primary, state as context. */
		p1 = symbol_ramp(r->symset, d.dom_frac);
		p2 = symbol_state(r->symset, d.dominant_state);
		break;
	case RENDER_DRAW:
		/* [draw_ramp][state] — draw density primary, state as context. */
		p1 = symbol_ramp(r->symset, d.draw_frac);
		p2 = symbol_state(r->symset, d.dominant_state);
		break;
	case RENDER_SUIT:
		/* [suit][state] — suit structure primary, state as context. */
		p1 = symbol_suit(r->symset, d.dominant_suit);
		p2 = symbol_state(r->symset, d.dominant_state);
		break;
	case RENDER_FLUSH:
		/* [flush_ramp][suit] — flush fraction primary, suit type as context. */
		p1 = symbol_ramp(r->symset, d.flush_frac);
		p2 = symbol_suit(r->symset, d.dominant_suit);
		break;
	case RENDER_EQUITY:
		/* [equity_ramp][suit] when available; fall through to JOINT otherwise. */
		if (s->equity >= 0.0f) {
			p1 = symbol_ramp(r->symset, (double)s->equity);
			p2 = symbol_suit(r->symset, d.dominant_suit);
			break;
		}
		/* fall through */
	case RENDER_JOINT:
	default:
		/* [state][suit_in_state] — both orthogonal dimensions, canonical joint. */
		p1 = symbol_state(r->symset, d.dominant_state);
		p2 = symbol_suit(r->symset, d.dominant_suit_in_state);
		break;
	}
	snprintf(buf, bufsz, "%s%s ", p1, p2);
}
