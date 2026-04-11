#include "symbols.h"

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

const char* symbol_empty(SymSet s) {
	return s == SYMSET_UNICODE ? "\xc2\xb7" : ".";
}
