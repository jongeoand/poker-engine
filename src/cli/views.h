#ifndef VIEWS_H_
#define VIEWS_H_

#include "render.h"
#include "symbols.h"
#include "map/handmap.h"
#include "htrange.h"
#include "analysis/combostate.h"
#include "panel.h"

/* 13×13 hand type range membership grid.
   Rows/cols from ACE(12) down to TWO(0); cells are 4 chars wide. */
TextPanel* views_htr_grid(const HandTypeRange* h);

/* Full field grid — respects r->mode, r->symset, r->width.
   sf is optional: pass a pre-built ScalarField when r->mode == RENDER_EQUITY
   so each cell's equity is drawn from it; pass NULL for all other modes
   (symbol_cell degrades RENDER_EQUITY to dom_frac when equity is absent). */
TextPanel* views_rangefield(Renderer* r, const RangeField* f, const ScalarField* sf);

/* Symbol legend for the current render mode and symbol set. */
TextPanel* views_legend(Renderer* r);

/* Combo state count summary block. */
TextPanel* views_state_summary(const ComboStateCounts* c);

/* ScalarField grid — renders each cell as a ramp symbol in [low→high].
   Cells with value < 0 (sentinel: no live combos) use symbol_empty.
   Uses r->symset; mode and width are ignored (always CELL_1 style). */
TextPanel* views_scalarfield(Renderer* r, const ScalarField* sf);

#endif
