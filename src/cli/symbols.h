#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include "render_types.h"
#include "analysis/combostate.h"
#include "map/handmap.h"

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

/* CellSample — bundles the two field values needed to render one grid cell.
   `cell`   is the packed joint distribution from the RangeField.
   `equity` is the precomputed mean villain equity from the ScalarField;
   set to -1.0f when no ScalarField is available (RENDER_EQUITY degrades
   gracefully to dom_frac, matching the ScalarField empty-cell sentinel). */
typedef struct {
    HMapCell cell;
    float    equity;
} CellSample;

/* Composite cell renderer — writes the symbol string(s) for one grid cell
   into buf according to r->mode and r->width.

   CELL_1: one glyph + one space  (2 display columns).
   CELL_2: two glyphs + one space  (3 display columns).
   CELL_4 is not handled here; views_rangefield manages label-based layout
   for that width directly and uses symbol_state for the trailing glyph.

   buf must be at least 16 bytes (safe for two UTF-8 glyphs + space + NUL).
   Handles empty cells and all currently defined RenderMode values. */
void symbol_cell(const CellSample* s, const Renderer* r, char* buf, size_t bufsz);

#endif
