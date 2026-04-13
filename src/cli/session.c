#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "session.h"
#include "command.h"
#include "views.h"
#include "panel.h"
#include "map/handmap.h"
#include "iterate.h"

// Session lifecycle

static const CommandTable session_table; /* forward declaration — defined below */

Session session_default(void) {
    Session s;
    s.game      = make_game(2);
    s.renderer  = render_default();
    
    // default rendering settings
    s.renderer.symset = SYMSET_UNICODE;
    s.renderer.width = CELL_2;

    s.has_hero         = false;
    s.has_board        = false;
    s.last_cards_dealt = 0;

    s.panel        = NULL;
    return s;
}

void session_free(Session* sesh) {
    for (int32_t i = 0; i < sesh->panel_count; i++)
        panel_free(sesh->panels[i]);
    free(sesh->panels);
    sesh->panels        = NULL;
}

void start_session(Session* sesh) {
    FILE* out = render_get_sink(&sesh->renderer);
    fprintf(out, "=== Session === (type 'help' for commands)\n\n");

	char line[256];
	while (1) {
		fprintf(out, "poker> ");
		fflush(out);
		if (!fgets(line, sizeof(line), stdin)) break;
		int rc = cmd_dispatch_line(&session_table, line, sesh);
		if (rc == CMD_QUIT) break;
	}

    session_free(sesh);
}

// Session specific helpers

/* Print the hero hand to the session sink. */
static void print_hero(Session* sesh) {
	FILE* out = render_get_sink(&sesh->renderer);
	fprintf(out, "hero: ");
	render_combo(&sesh->renderer, sesh->game.playerhands[0]);
	fprintf(out, "\n");
}

/* Print the current board to the session sink. */
static void print_board(Session* sesh) {
	FILE* out = render_get_sink(&sesh->renderer);
	if (sesh->game.board == 0) {
		fprintf(out, "board: (none)\n");
		return;
	}
	fprintf(out, "board: ");
	render_board(&sesh->renderer, sesh->game.board);
	fprintf(out, "\n");
}

// ------------------------------------------------------------------ 
// Command handlers                                                     
// ------------------------------------------------------------------ 

static int cmd_deal(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    deal_players(&sesh->game);

    sesh->last_cards_dealt = combo_toBitmask(sesh->game.playerhands[0]);
    sesh->has_hero = true;

    print_hero(sesh);
    return CMD_OK;
}

static int cmd_street(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    uint64_t before = sesh->game.board;
    deal_street(&sesh->game);
    sesh->last_cards_dealt = sesh->game.board & ~before;

    print_board(sesh);
    return CMD_OK;
}

static int cmd_undodeal(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    FILE* out = render_get_sink(&sesh->renderer);

    if (!sesh->has_hero || sesh->last_cards_dealt == 0) {
        fprintf(out, "nothing to undo\n");
        return CMD_ERR;
    }

    if (sesh->has_board) {
        /* Undo the last street: return its cards to the deck and strip from board. */
        sesh->game.deck  |=  sesh->last_cards_dealt;
        sesh->game.board &= ~sesh->last_cards_dealt;
        sesh->last_cards_dealt = 0;
        if (sesh->game.board == 0)
            sesh->has_board = false;
        print_board(sesh);
    } else {
        /* Undo deal_players: reset the game entirely so the full deck is restored
           (villain's cards are also removed from the deck by deal_players and are
           not tracked individually, so make_game is the only clean restore path). */
        sesh->game             = make_game(sesh->game.headcount);
        sesh->last_cards_dealt = 0;
        sesh->has_hero         = false;
        fprintf(out, "hero hand cleared\n");
    }

    return CMD_OK;
}

// print hero combo if Session has_hero, otherwise deal hero hand and print to sink
static int cmd_hero(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    
    if (sesh->has_hero) { print_hero(sesh); return CMD_OK; }

    deal_players(&sesh->game);
    sesh->has_hero = true;
    print_hero(sesh);

    return CMD_OK;
}

// print board if Session has one otherwise deal flop
static int cmd_board(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    
    if (sesh->has_board) { print_board(sesh); return CMD_OK; }

    if (!sesh->has_hero) { deal_players(&sesh->game); }

    deal_street(&sesh->game);
    sesh->has_board = true;
    sesh->last_cards_dealt = sesh->game.board;
    print_board(sesh);

    return CMD_OK;
}

// invariants - one street is dealt and hero has hand  
static int cmd_combostream(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    bool streamable = sesh->has_hero && sesh->has_board;

    if (!streamable) { return CMD_ERR; }

    HtrComboStream stream;
    HandTypeRange full = htr_full();

    Combo    hero_combo = sesh->game.playerhands[0];
    uint64_t hero = toBitmask(hero_combo.a) | toBitmask(hero_combo.b);
    uint64_t dead = sesh->game.board | hero;

    combostream_init(&stream, &full, dead);

    Combo current;

    while (combostream_next(&stream, &current)) {
        render_combo(&sesh->renderer, current);
        render_blank(&sesh->renderer);
    }

    return CMD_OK;
}

static int cmd_render_settings(Session* sesh, int argc, char** argv) {
    FILE* out = render_get_sink(&sesh->renderer);

    // no args - print current symset and width
    if (argc == 0) {
        fprintf(out, "symset: %s  width: %d\n",
                sesh->renderer.symset == SYMSET_UNICODE ? "unicode" : "ascii",
                sesh->renderer.width == CELL_1 ? 1 :
                sesh->renderer.width == CELL_2 ? 2 : 4);
        return CMD_OK;
    }
    for (int i = 0; i < argc; i++) {
        const char* a = argv[i];
        if      (strcmp(a, "unicode") == 0) sesh->renderer.symset = SYMSET_UNICODE;
        else if (strcmp(a, "ascii")   == 0) sesh->renderer.symset = SYMSET_ASCII;
        else if (strcmp(a, "1") == 0)       sesh->renderer.width  = CELL_1;
        else if (strcmp(a, "2") == 0)       sesh->renderer.width  = CELL_2;
        else if (strcmp(a, "4") == 0)       sesh->renderer.width  = CELL_4;
        else fprintf(out, "unknown render setting '%s'  (unicode | ascii | 1 | 2 | 4)\n", a);
    }
    return CMD_OK;
}

static int cmd_analyze(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    FILE* out = render_get_sink(&sesh->renderer);

    // deal hero and board if they don't exist yet
    if (!sesh->has_hero)  { deal_players(&sesh->game);  sesh->has_hero  = true; }
    if (!sesh->has_board) { deal_street(&sesh->game);   sesh->has_board = true; }

    Combo    hero    = sesh->game.playerhands[0];
    uint64_t bitmask = toBitmask(hero.a) | toBitmask(hero.b);
    uint64_t dead    = bitmask | sesh->game.board;

    HandTypeRange full = htr_full();

    RangeField range_field = hmap_build(&full, dead, sesh->game.board, bitmask);
    StateField plane;

    hmap_project_state(&range_field, &plane);

    fprintf(out, "RangeField: \n");
    { TextPanel* _p = views_rangefield(&sesh->renderer, &range_field); panel_print(_p, &sesh->renderer); panel_free(_p); }
    render_blank(&sesh->renderer);

    fprintf(out, "StateField: \n");
    { TextPanel* _p = views_statefield(&sesh->renderer, &plane); panel_print(_p, &sesh->renderer); panel_free(_p); }
    return CMD_OK;
}

static int cmd_project(Session* sesh, int argc, char** argv) {
    FILE* out = render_get_sink(&sesh->renderer);
    int n = 1;

    if (argc == 0) {
        if (!sesh->has_hero)  { deal_players(&sesh->game);  sesh->has_hero  = true; }
        if (!sesh->has_board) { deal_street(&sesh->game);   sesh->has_board = true; }
    } else {
        n = atoi(argv[0]);
        if (n <= 0) { fprintf(out, "invalid count '%s'\n", argv[0]); return CMD_ERR; }
    }

    Combo    hero    = sesh->game.playerhands[0];
    uint64_t bitmask = toBitmask(hero.a) | toBitmask(hero.b);
    uint64_t dead    = bitmask | sesh->game.board;

    HandTypeRange full = htr_full();

    RangeField range_field = hmap_build(&full, dead, sesh->game.board, bitmask);
    StateField plane;

    hmap_project_state(&range_field, &plane);

    fprintf(out, "RangeField: \n");
    { TextPanel* _p = views_rangefield(&sesh->renderer, &range_field); panel_print(_p, &sesh->renderer); panel_free(_p); }
    render_blank(&sesh->renderer);

    for (int i = 0; i < n; i++) {
        fprintf(out, "StateField %d \n", i);
        { TextPanel* _p = views_statefield(&sesh->renderer, &plane); panel_print(_p, &sesh->renderer); panel_free(_p); }
    }

    return CMD_OK;
}

/* reset  — fresh deck, cleared board and hands, cleared panels */
static int cmd_reset(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	sesh->game      = make_game(2);
	sesh->has_hero  = false;
	sesh->has_board = false;
	session_free(sesh);
	fprintf(render_get_sink(&sesh->renderer), "session reset\n");
	return CMD_OK;
}

/* help  — print command table */
static int cmd_help(Session* sesh, int argc, char** argv);

/* quit  — exit session */
static int cmd_quit(Session* sesh, int argc, char** argv) {
	(void)sesh; (void)argc; (void)argv;
	return CMD_QUIT;
}

/* ------------------------------------------------------------------ */
/* Command table                                                        */
/* ------------------------------------------------------------------ */

static const Command session_cmds[] = {
	{ "deal",    'd', "deal",                        "deal hero hand randomly from deck",                     cmd_deal           },
	{ "undodeal",'u', "undodeal",                    "undo last deal action",                                 cmd_undodeal       },
	{ "street",  's', "street",                      "deal next community street from deck",                  cmd_street         },
	{ "print hero",  'h', "print hero",  "print hero hand",      cmd_hero  },
	{ "print board", 'b', "print board", "print cards on board", cmd_board },
	{ "stream",  'v', "stream",                       "stream all villain combos vs hero + board",             cmd_combostream    },
	{ "render",  'R', "render [unicode|ascii|1|2|4]","toggle renderer settings",                              cmd_render_settings},
	{ "analyze",  'a', "analyze",                      "build RangeField + StateField vs hero + board",         cmd_analyze        },
	{ "project",  'p', "project [n]",                 "project StateField n times (default 1)",                cmd_project        },
	{ "reset",   'c', "reset",                       "clear all session state (deck, range)",                 cmd_reset          },
	{ "help",    '?', "help",                        "print this command list",                               cmd_help           },
	{ "quit",    'q', "quit",                        "exit session",                                          cmd_quit           },
};

static const CommandTable session_table = {
	session_cmds,
	(int)(sizeof(session_cmds) / sizeof(session_cmds[0]))
};

/* Defined after table so cmd_help can reference it. */
static int cmd_help(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	cmd_print_help(&session_table, render_get_sink(&sesh->renderer));
	return CMD_OK;
}

