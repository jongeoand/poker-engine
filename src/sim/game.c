#include <string.h>
#include "game.h"

Game make_game(uint8_t headcount) {
	Game g;
	memset(&g, 0, sizeof(g));
	g.headcount = headcount;
	g.deck      = FULL_DECK;
	return g;
}

void deal_players(Game* game) {
	for (int round = 0; round < 2; round++) {
		for (int player = 0; player < game->headcount; player++) {
			uint64_t cardbit = deal(&game->deck);
			if (round == 0)
				game->playerhands[player].a = toCard(cardbit);
			else
				game->playerhands[player].b = toCard(cardbit);
		}
	}
}

void deal_street(Game* game) {
	int n;
	switch (__builtin_popcountll(game->board)) {
		case 0: n = 3; break;  // flop
		case 3: n = 1; break;  // turn
		case 4: n = 1; break;  // river
		default: return;       // river already dealt, or invalid state
	}
	for (int i = 0; i < n; i++)
		game->board |= deal(&game->deck);
}

void deal_bomb(Game* game) {
	deal_players(game);
	deal_street(game);
}

void deal_hand(Game* game) {
	if(game->deck == FULL_DECK) {
		deal_players(game);
	}

	while(__builtin_popcountll(game->board) < 5) {
		deal_street(game);
		output_board(game->board); printf("\n");
	}
}

int get_street(Game* game) {
	int card_count = __builtin_popcountll(game->board);

	if (card_count > 0) {
		if (card_count == 3) { return 1; } // flop
		if (card_count == 4) { return 2; } // turn
		if (card_count == 5) { return 3; } // river
	}

	return 0; // preflop
}
