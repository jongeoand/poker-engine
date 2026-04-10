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


const char* combostate_str(ComboState s);
char combostate_symbol(ComboState s);

ComboState classify_combostate(uint64_t board, uint64_t hero, uint64_t villain);

// Aggregate struct for many classify_combostate from stream
typedef struct {
	int counts[COMBO_STATE_COUNT];
	int total;
} ComboStateCounts;

ComboStateCounts count_combostates(const HandTypeRange* r, uint64_t board, uint64_t hero, uint64_t dead);
#endif
