#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "session.h"
#include "command.h"
#include "views.h"
#include "map/handmap.h"

/* ------------------------------------------------------------------ */
/* Input parsers                                                        */
/* ------------------------------------------------------------------ */

/* Parse a hand type token: "AKs", "AKo", "QQ", "AK" (defaults offsuit).
   Returns true on success, false on invalid input. */
static bool parse_handtype(const char* s, HandType* out) {
	if (!s || !s[0] || !s[1]) return false;
	int r1 = rank_from_char(s[0]);
	int r2 = rank_from_char(s[1]);
	if (r1 < 0 || r2 < 0) return false;

	char suffix = s[2];
	if (suffix != '\0' && s[3] != '\0') return false; /* extra chars */

	if (r1 == r2) {
		if (suffix == 's' || suffix == 'o') return false; /* pairs can't be suited/offsuit */
		*out = make_pair((uint8_t)r1);
		return true;
	}

	uint8_t hi = (r1 > r2) ? (uint8_t)r1 : (uint8_t)r2;
	uint8_t lo = (r1 < r2) ? (uint8_t)r1 : (uint8_t)r2;

	if (suffix == 's') {
		*out = make_suited(hi, lo);
	} else {
		/* 'o' or no suffix — default to offsuit */
		*out = make_offsuit(hi, lo);
	}
	return true;
}

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

/* range [AKs QQ ...]  — add hand types; no args = show current grid */
static int cmd_range(Session* sesh, int argc, char** argv) {
	if (argc == 0) {
		HandTypeRange full = htr_full();
        sesh->villain_range = full;
        return CMD_OK;
	}
	FILE* out = render_get_sink(&sesh->renderer);
	int added = 0;
	for (int i = 0; i < argc; i++) {
		HandType ht;
		if (!parse_handtype(argv[i], &ht)) {
			fprintf(out, "invalid hand type: '%s'\n", argv[i]);
			continue;
		}
		htr_add(&sesh->villain_range, ht);
		added++;
	}
	if (added) fprintf(out, "villain range: %d type(s) set\n", htr_count(&sesh->villain_range));
	return CMD_OK;
}

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

/* hand  — deal players + all streets */
static int cmd_hand(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	if (sesh->game.deck == FULL_DECK)
		deal_players(&sesh->game);
	while (__builtin_popcountll(sesh->game.board) < 5)
		deal_street(&sesh->game);
	print_hero(sesh);
	print_board(sesh);
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
	views_htr_grid(&sesh->renderer, &sesh->villain_range);
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
	if (htr_count(&sesh->villain_range) == 0) {
		fprintf(out, "villain range is empty — use 'range' first\n");
		return CMD_ERR;
	}

	Combo    hero  = sesh->game.playerhands[0];
	uint64_t hmask = toBitmask(hero.a) | toBitmask(hero.b);
	uint64_t dead  = hmask | sesh->game.board;

	RangeField rf = hmap_build(&sesh->villain_range, dead, sesh->game.board, hmask);
	StateField sf;
	hmap_project_state(&rf, &sf);

	views_rangefield(&sesh->renderer, &rf);
	render_blank(&sesh->renderer);
	views_statefield(&sesh->renderer, &sf);
	return CMD_OK;
}

/* reset  — fresh deck, cleared board and hands, empty range */
static int cmd_reset(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	sesh->game          = make_game(2);
	sesh->villain_range = htr_empty();
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
	{ "range",   'r', "range [AKs QQ ...]",        "add hand types to villain range (no args = show grid)", cmd_range          },
	{ "deal",    'd', "deal",                        "deal hero hand randomly from deck",                     cmd_deal           },
	{ "street",  's', "street",                      "deal next community street from deck",                  cmd_street         },
	{ "hand",    'H', "hand",                        "deal hero + all streets",                               cmd_hand           },
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
	s.villain_range = htr_empty();
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
