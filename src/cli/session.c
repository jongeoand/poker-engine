#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "session.h"
#include "command.h"
#include "views.h"
#include "map/handmap.h"

/* ------------------------------------------------------------------ */
/* Session state helpers                                                */
/* ------------------------------------------------------------------ */

/* True if at least one card has been removed from the deck. */
static bool session_cards_dealt(const Session* sesh) {
	return sesh->game.deck != FULL_DECK;
}

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

/* ------------------------------------------------------------------ */
/* Command handlers                                                     */
/* ------------------------------------------------------------------ */

/* deal  — deal_players: randomly deal hero (and villain slot) from deck */
static int cmd_deal(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	deal_players(&sesh->game);
	print_hero(sesh);
	return CMD_OK;
}

/* street  — deal next community street from deck */
static int cmd_street(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	deal_street(&sesh->game);
	print_board(sesh);
	return CMD_OK;
}

static int cmd_hero(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	if (session_cards_dealt(sesh)) {
		print_hero(sesh);
	} else {
		fprintf(render_get_sink(&sesh->renderer), "No hero hand dealt yet\n");
	}
	return CMD_OK;
}

static int cmd_board(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	FILE* out = render_get_sink(&sesh->renderer);
	if (sesh->game.board != 0) { print_board(sesh); }
	else { fprintf(out, "No street dealt yet\n"); }
	return CMD_OK;
}
/* view [state|purity|draw]  — render villain range grid */
static int cmd_view(Session* sesh, int argc, char** argv) {
	if (argc > 0) {
		if      (strcmp(argv[0], "state")  == 0) sesh->renderer.mode = RENDER_STATE;
		else if (strcmp(argv[0], "purity") == 0) sesh->renderer.mode = RENDER_PURITY;
		else if (strcmp(argv[0], "draw")   == 0) sesh->renderer.mode = RENDER_DRAW;
		else {
			fprintf(render_get_sink(&sesh->renderer),
			        "unknown view mode '%s'  (state | purity | draw)\n", argv[0]);
			return CMD_ERR;
		}
	}
	HandTypeRange full = htr_full();
	views_htr_grid(&sesh->renderer, &full);
	return CMD_OK;
}

/* render [unicode|ascii|1|2|4]  — toggle renderer settings */
static int cmd_render_settings(Session* sesh, int argc, char** argv) {
	FILE* out = render_get_sink(&sesh->renderer);
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

/* analyze  — build RangeField + StateField vs hero hand + board; render */
static int cmd_analyze(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	FILE* out = render_get_sink(&sesh->renderer);

	if (!session_cards_dealt(sesh)) {
		fprintf(out, "no hero hand — use 'deal' first\n");
		return CMD_ERR;
	}

	Combo    hero  = sesh->game.playerhands[0];
	uint64_t hmask = toBitmask(hero.a) | toBitmask(hero.b);
	uint64_t dead  = hmask | sesh->game.board;

    HandTypeRange villain_range = htr_full();

	RangeField rf = hmap_build(&villain_range, dead, sesh->game.board, hmask);
	StateField sf;
	hmap_project_state(&rf, &sf);
    
    fprintf(out, "RangeField: \n");
	views_rangefield(&sesh->renderer, &rf);
	render_blank(&sesh->renderer);

    fprintf(out, "StateField: \n");
	views_statefield(&sesh->renderer, &sf);
	return CMD_OK;
}

/* reset  — fresh deck, cleared board and hands, empty range */
static int cmd_reset(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	sesh->game = make_game(2);
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
	{ "street",  's', "street",                      "deal next community street from deck",                  cmd_street         },
	{ "print hero",  'h', "print hero",  "print hero hand",      cmd_hero  },
	{ "print board", 'b', "print board", "print cards on board", cmd_board },
    { "view",    'v', "view [state|purity|draw]",    "render villain range grid",                             cmd_view           },
	{ "render",  'R', "render [unicode|ascii|1|2|4]","toggle renderer settings",                              cmd_render_settings},
	{ "analyze", 'a', "analyze",                     "build RangeField + StateField vs hero + board",         cmd_analyze        },
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

/* ------------------------------------------------------------------ */
/* Session lifecycle                                                    */
/* ------------------------------------------------------------------ */

Session session_default(void) {
	Session s;
	s.game          = make_game(2);
	s.renderer      = render_default();
	return s;
}

void start_session(Session* sesh) {
	FILE* out = render_get_sink(&sesh->renderer);
	fprintf(out, "=== Session ===  (type 'help' for commands, 'quit' to exit)\n\n");

	char line[256];
	while (1) {
		fprintf(out, "poker> ");
		fflush(out);
		if (!fgets(line, sizeof(line), stdin)) break;
		int rc = cmd_dispatch_line(&session_table, line, sesh);
		if (rc == CMD_QUIT) break;
	}
}
