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

// params: 
//  game - the game context
//  i - player pos
//
// returns the HandTypeRange that has OESD and gutshots from the range that the player in position i is ahead of
HandTypeRange straight_draws(Game* game, int i) {
	HandTypeRange draws = aheadof(game, i);

	uint8_t drawflags = DRAW_OESD | DRAW_GUTSHOT;
	draws = htrfilter_by_draw(&draws, game->board, drawflags);
	return draws;
}
