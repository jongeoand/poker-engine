#ifndef COMBOSTATE_H
#define COMBOSTATE_H

#include <stdint.h>

#include "core/combo.h"

typedef enum {
	COMBO_AHEAD,
	COMBO_CHOP,
	COMBO_BEHIND_LIVE,
	COMBO_BEHIND_DEAD,
	COMBO_STATE_COUNT,
} ComboState;

// Suit texture of a villain combo relative to the board.
// Ordered by increasing strength so comparisons (sc_a > sc_b) are meaningful.
// Constraint: SUIT_CLASS_COUNT == 4 — the packed HMapCell nibble layout depends on this.
typedef enum {
	SUIT_NONE = 0,      // no flush interest — no suit cluster of 3+
	SUIT_BACKDOOR,      // 3 cards of one suit including ≥1 hole card (backdoor draw)
	SUIT_FLUSH_DRAW,    // 4 cards of one suit including ≥1 hole card (~35% equity)
	SUIT_FLUSH_MADE,    // 5 cards of one suit including ≥1 hole card (made flush)
	SUIT_CLASS_COUNT,
} SuitClass;


const char* combostate_str(ComboState s);
char combostate_symbol(ComboState s);

ComboState classify_combostate(uint64_t board, uint64_t hero, uint64_t villain);

// Classify the suit texture of `combo` relative to `board`.
// Returns the strongest SuitClass the combo participates in across all suits.
// Only counts a suit if the combo contributes at least one card to it.
// card bit layout: suit s occupies bits [s*13, s*13+12] (clubs=0, diamonds=1, hearts=2, spades=3)
SuitClass classify_suit(uint64_t board, uint64_t combo);

// Aggregate struct for many classify_combostate from stream
typedef struct {
	int counts[COMBO_STATE_COUNT];
	int total;
} ComboStateCounts;

ComboStateCounts count_combostates(const HandTypeRange* r, uint64_t board, uint64_t hero, uint64_t dead);
#endif
