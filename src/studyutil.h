#ifndef STUDYUTIL_H_
#define STUDYUTIL_H_

#include "game.h"

HandTypeRange aheadof(Game* g, int i);

HandTypeRange behind(Game* g, int i);

typedef struct {
	Game g;
	int position;
	Combo hero;
} StudySpot;


#endif
