#ifndef VIEWS_H_
#define VIEWS_H_

#include "render.h"
#include "symbols.h"
#include "map/handmap.h"
#include "htrange.h"
#include "analysis/combostate.h"

/* 13×13 hand type range membership grid.
   Rows/cols from ACE(12) down to TWO(0); cells are 4 chars wide. */
void views_htr_grid(Renderer* r, const HandTypeRange* h);

/* Full topology grid — respects r->mode, r->symset, r->width. */
void views_rangefield(Renderer* r, const RangeField* f);

/* Dominant-state-only grid — respects r->symset. */
void views_statefield(Renderer* r, const StateField* f);

/* Hand rank distribution table. */
void views_board_profile(Renderer* r, const HtrBoardProfile* p);

/* Symbol legend for the current render mode and symbol set. */
void views_legend(Renderer* r);

/* Combo state count summary block. */
void views_state_summary(Renderer* r, const ComboStateCounts* c);

#endif
