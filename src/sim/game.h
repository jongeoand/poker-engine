#ifndef GAME_H_
#define GAME_H_

#include "combo.h"
#include "engine.h"

#define MAX_PLAYERS 9

typedef struct {
	uint8_t headcount;
	Combo   playerhands[MAX_PLAYERS];
	uint64_t board;
	uint64_t deck;
} Game;

Game make_game(uint8_t headcount);

void deal_players(Game* game);

void deal_street(Game* game);

//deals pre and post flop
void deal_bomb(Game* game);

// deals an entire hand - all players receive cards and all three streets are dealt
void deal_hand(Game* game);

int get_street(Game* game);
#endif
