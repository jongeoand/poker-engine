#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include "render.h"
#include "analysis/combostate.h"

/* State symbol — the primary character encoding a ComboState in a grid cell.
   Returns a NUL-terminated string (1 byte ASCII, 3 bytes UTF-8 for Unicode). */
const char* symbol_state(SymSet s, ComboState state);

/* Purity/intensity ramp — encodes a [0,1] fraction as a stepped character.
   Returns a NUL-terminated string. */
const char* symbol_ramp(SymSet s, double frac);

/* Empty-cell placeholder for each symbol set. */
const char* symbol_empty(SymSet s);

#endif
