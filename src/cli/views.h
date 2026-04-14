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

/* Full field grid — respects r->mode, r->symset, r->width. */
TextPanel* views_rangefield(Renderer* r, const RangeField* f);

/* Symbol legend for the current render mode and symbol set. */
TextPanel* views_legend(Renderer* r);

/* Combo state count summary block. */
TextPanel* views_state_summary(const ComboStateCounts* c);

#endif
