#ifndef SESSION_H_
#define SESSION_H_

#include <stdint.h>
#include <stdbool.h>

#include "htrange.h"
#include "game.h"
#include "render.h"
#include "panel.h"

/* Interactive session state.
   game.headcount = 2; game.playerhands[0] = hero hand.
   Board and hero are accessed via game.board and game.playerhands[0].
   deck != FULL_DECK indicates at least one card has been dealt. */
typedef struct Session {
	Game          game;
	Renderer      renderer;

    // vars for tracking session state
    bool has_hero;
    bool has_board;

    uint64_t last_cards_dealt;

    uint64_t    street_boards[3];   /* cumulative board mask after each street: [0]=flop, [1]=turn, [2]=river */
    int         street_count;       /* number of streets dealt (0–3) */
} Session;

/* Initialize a default session: 2-player game, default renderer, empty range. */
Session session_default(void);

/* Start the interactive REPL loop. Returns when the user quits. */
void start_session(Session* sesh);

#endif
