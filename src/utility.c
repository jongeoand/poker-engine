#include "utility.h"

// Return the HandType range that the player in position i in game is ahead of.
HandTypeRange aheadof(Game* game, int i) {
	Combo hero = game->playerhands[i];
	uint64_t bitmask = combo_toBitmask(hero);
	
	HandTypeRange range = htr_full();
	
	return htrfilter_ahead(game->board, bitmask, &range);	
} 

// Return the HandType range that the player in position i in game is behind
HandTypeRange behind(Game* game, int i) {
	Combo hero = game->playerhands[i];
	uint64_t bitmask = combo_toBitmask(hero);
	
	HandTypeRange range = htr_full();
	return htrfilter_behind(game->board, bitmask, &range);
}

// Profile the range against the current game board
HtrBoardProfile profile(const HandTypeRange* range, Game* game) {
	return htr_board_profile(range, game->board);
}
