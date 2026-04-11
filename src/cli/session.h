#ifndef SESSION_H_
#define SESSION_H_

#include "htrange.h"
#include "game.h"
#include "render.h"

/* Interactive session state.
   game.headcount = 2; game.playerhands[0] = hero hand.
   Board and hero are accessed via game.board and game.playerhands[0].
   deck != FULL_DECK indicates at least one card has been dealt. */
typedef struct Session {
	HandTypeRange villain_range;
	Game          game;
	Renderer      renderer;
} Session;

/* Initialize a default session: 2-player game, default renderer, empty range. */
Session session_default(void);

/* Start the interactive REPL loop. Returns when the user quits. */
void start_session(Session* sesh);

#endif
