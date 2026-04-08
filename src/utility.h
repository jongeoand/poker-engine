#ifndef UTILITY_H_
#define UTILITY_H_

#include "game.h"

HandTypeRange aheadof(Game* game, int i);

HandTypeRange behind(Game* game, int i);

HtrBoardProfile profile(const HandTypeRange* range, Game* game);
#endif
