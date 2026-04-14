#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include "render_types.h"
#include "analysis/combostate.h"

/* State symbol — the primary character encoding a ComboState in a grid cell.
   Returns a NUL-terminated string (1 byte ASCII, 3 bytes UTF-8 for Unicode). */
const char* symbol_state(SymSet s, ComboState state);

/* Purity/intensity ramp — encodes a [0,1] fraction as a stepped character.
   Returns a NUL-terminated string. */
const char* symbol_ramp(SymSet s, double frac);

/* Suit symbol — encodes a SuitClass using a circle-fill progression,
   perceptually distinct from the block-family state symbols.
   ASCII: n b f F  (none / backdoor / flush-draw / flush-made)
   Unicode: ○ ◑ ◕ ●  (U+25CB / U+25D1 / U+25D5 / U+25CF) */
const char* symbol_suit(SymSet s, SuitClass suit);

/* Empty-cell placeholder for each symbol set. */
const char* symbol_empty(SymSet s);

#endif
