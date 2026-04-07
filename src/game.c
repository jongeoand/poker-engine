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

		print_playerhands(game); printf("\n");
	}
}

void print_playerhands(Game* game) {
	for (int i = 0; i < game->headcount; i++) {
		Combo hand = game->playerhands[i];
		uint64_t mask = combo_toBitmask(hand);

		printf(" Player %d: ", i);
		output_combo(hand); printf(" ");
		printf("0x%08X  ", calculate_hand_strength(game->board | mask));
		output_hand_rank(classify_hand(game->board | mask)); printf("\n");
	}
}

void print_playerpov(Game* game, int i) {
	Combo hand = game->playerhands[i];

	HandTypeRange fullhtr = htr_full();
	HtrEquitySplit matchups = htr_vs_combo(game->board, combo_toBitmask(hand), &fullhtr);
	output_htr_equity_split(&matchups);
}
