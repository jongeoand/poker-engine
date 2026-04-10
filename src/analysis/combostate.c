#include "analysis/combostate.h"

const char* combostate_str(ComboState s) {
	switch (s) {
		case COMBO_AHEAD:       return "ahead";
		case COMBO_CHOP:        return "chop";
		case COMBO_BEHIND_LIVE: return "behind_live";
		case COMBO_BEHIND_DEAD: return "behind_dead";
		default:                return "unknown";
	}
}

char combostate_symbol(ComboState s) {
	switch (s) {
		case COMBO_AHEAD:       return 'A';
		case COMBO_CHOP:        return 'C';
		case COMBO_BEHIND_LIVE: return 'L';
		case COMBO_BEHIND_DEAD: return 'D';
		default:                return '?';
	}
}

ComboState classify_combostate(uint64_t board, uint64_t hero, uint64_t villain) {
	int comparison = compare_hands(board, hero, villain);

	// villain beats hero
	if (comparison < 0) return COMBO_AHEAD;

	// tie
	if (comparison == 0) return COMBO_CHOP;

	// hero beats villain — check if villain has live draws
	DrawInfo d = compute_draws(board, villain);
	if (d.flags & (DRAW_FLUSH_DRAW | DRAW_OESD | DRAW_GUTSHOT | DRAW_COMBO_DRAW)) {
		return COMBO_BEHIND_LIVE;
	}

	return COMBO_BEHIND_DEAD;
}

ComboStateCounts count_combostates(const HandTypeRange* r, uint64_t board, uint64_t hero, uint64_t dead) {
	ComboStateCounts bucket = {0};
	HtrComboStream stream;
	Combo hand;

	combostream_init(&stream, r, dead);

	while (combostream_next(&stream, &hand)) {
		ComboState state = classify_combostate(board, hero, combo_toBitmask(hand));
		bucket.counts[state]++;
		bucket.total++;
	}

	return bucket;
}
