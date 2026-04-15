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

    s.street_count = 0;
    return s;
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

}

// ------------------------------------------------------------------
// Ops — all Session mutation lives here
// ------------------------------------------------------------------


int op_reset(Session* s) {
    s->game         = make_game(2);
    s->has_hero     = false;
    s->has_board    = false;
    s->street_count = 0;
    return CMD_OK;
}

int op_dealhero(Session* s) {
    if (s->has_hero) { op_reset(s); }

    deal_players(&s->game);
    s->last_cards_dealt = combo_toBitmask(s->game.playerhands[0]);
    s->has_hero = true;
    return CMD_OK;
}

int op_assignhero(Session* s, Combo c) {
    s->game.playerhands[0] = c;
    s->last_cards_dealt = combo_toBitmask(c);
    s->game.deck &= ~s->last_cards_dealt;
    s->has_hero = true;
    return CMD_OK;
}

int op_assignvillain(Session* s, Combo c) {
    s->game.playerhands[1] = c;
    return CMD_OK;
}

int op_assignboard(Session* s, uint64_t cardmask) {
    s->game.board |= cardmask;
    s->last_cards_dealt = cardmask;
    s->game.deck &= ~s->last_cards_dealt;
    return CMD_OK;
}

int op_dealstreet(Session* s) {
    if (s->street_count >= 3) return CMD_ERR;
    uint64_t before = s->game.board;
    deal_street(&s->game);
    s->last_cards_dealt = s->game.board & ~before;
    s->street_boards[s->street_count++] = s->game.board;
    s->has_board = true;
    return CMD_OK;
}

int op_dealbomb(Session* s) {
    deal_bomb(&s->game);
    s->last_cards_dealt = s->game.board;
    s->has_hero  = true;
    s->street_boards[s->street_count++] = s->game.board;
    s->has_board = true;
    return CMD_OK;
}

int op_dealthru(Session* s) {
    op_ensure_hero(s);
    op_ensure_board(s);
   
    s->street_boards[s->street_count++] = s->game.board;

    for (int street = 1; street < 3; street++) {
        deal_street(&s->game);
        s->street_boards[s->street_count++] = s->game.board;
    }
    
    s->last_cards_dealt = s->street_boards[s->street_count];
    return CMD_OK;
}

int op_undo(Session* s) {
    if (!s->has_hero || s->last_cards_dealt == 0) return CMD_ERR;
    if (s->has_board) {
        s->game.deck  |=  s->last_cards_dealt;
        s->game.board &= ~s->last_cards_dealt;
        s->last_cards_dealt = 0;
        if (s->street_count > 0) s->street_count--;
        if (s->game.board == 0) s->has_board = false;
    } else {
        s->game             = make_game(s->game.headcount);
        s->last_cards_dealt = 0;
        s->has_hero         = false;
    }
    return CMD_OK;
}

int op_ensure_hero(Session* s) {
    if (!s->has_hero) return op_dealhero(s);
    return CMD_OK;
}

int op_ensure_board(Session* s) {
    if (!s->has_board) return op_dealstreet(s);
    return CMD_OK;
}

// ------------------------------------------------------------------
// Context 
// ------------------------------------------------------------------

Context get_context(const Session* sesh) {
    uint64_t hero_mask = combo_toBitmask(sesh->game.playerhands[0]);
    return (Context){ hero_mask, hero_mask | sesh->game.board, sesh->game.board };
}

Context get_vcontext(const Session* sesh) {
    uint64_t villain_mask = combo_toBitmask(sesh->game.playerhands[1]);
    return (Context){ villain_mask, villain_mask | sesh->game.board, sesh->game.board };
}

// Windows 

TextPanel* make_layout(const Session* sesh, Context ctx) {
    HandTypeRange full = htr_full();
    
    TextPanel* layout = NULL;
    Renderer* r = (Renderer*)&sesh->renderer;

    for (int street = 0; street < sesh->street_count; street++) {
        uint64_t board = sesh->street_boards[street];
        RangeField matrix = hmap_build(&full, ctx.dead, board, ctx.hero_mask);
        
        TextPanel* view;

        if (sesh->renderer.mode != RENDER_EQUITY) { view = views_rangefield(r, &matrix, NULL); }
        else {
            ScalarField sf = scalar_build(&matrix, ctx.dead, board, ctx.hero_mask);
            view = views_rangefield(r, &matrix, &sf);
        }
        
        layout = (layout == NULL) ? view : panel_join_consume(layout, view, 1);
    }
    
    TextPanel* legend = views_legend(r);
    if (layout == NULL) return legend;
    layout = panel_stack_consume(legend, layout, 1);
    return layout;
}

TextPanel* make_dashboard(const Session* sesh, Context ctx) {
    HandTypeRange full = htr_full();

    TextPanel* dashboard = NULL;

    Renderer r = render_default();
    r.width  = CELL_2;
    r.symset = SYMSET_UNICODE;

    RenderMode modes[] = {RENDER_PURITY, RENDER_EQUITY, RENDER_DRAW, RENDER_FLUSH, RENDER_JOINT};
    int nmode = (int)(sizeof(modes) / sizeof(modes[0]));

    for (int i = 0; i < nmode; i++) {
        r.mode = modes[i];

        TextPanel* row = NULL;

        for (int street = 0; street < sesh->street_count; street++) {
            uint64_t board = sesh->street_boards[street];
            RangeField matrix = hmap_build(&full, ctx.dead, board, ctx.hero_mask);

            TextPanel* view;
            if (r.mode != RENDER_EQUITY) { view = views_rangefield(&r, &matrix, NULL); }
            else {
                ScalarField sf = scalar_build(&matrix, ctx.dead, board, ctx.hero_mask);
                view = views_rangefield(&r, &matrix, &sf);
            }

            row = (row == NULL) ? view : panel_join_consume(row, view, 1);
        }

        TextPanel* legend = views_legend(&r);
        row = (row == NULL) ? legend : panel_join_consume(row, legend, 1);

        dashboard = (dashboard == NULL) ? row : panel_stack_consume(dashboard, row, 2);
    }

    return dashboard;
}

// ------------------------------------------------------------------
// Print helpers
// ------------------------------------------------------------------

static void print_hero(Session* sesh) {
	FILE* out = render_get_sink(&sesh->renderer);
	fprintf(out, "hero: ");
	render_combo(&sesh->renderer, sesh->game.playerhands[0]);
	fprintf(out, "\n");
}

static void print_villain(Session* sesh) {
	FILE* out = render_get_sink(&sesh->renderer);
	fprintf(out, "villain: ");
	render_combo(&sesh->renderer, sesh->game.playerhands[1]);
	fprintf(out, "\n");
}

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
    op_dealhero(sesh);
    print_hero(sesh);
    return CMD_OK;
}

static int cmd_street(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    if (op_dealstreet(sesh) != CMD_OK) {
        fprintf(render_get_sink(&sesh->renderer), "all streets dealt\n");
        return CMD_ERR;
    }
    print_board(sesh);
    return CMD_OK;
}

static int cmd_undodeal(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    FILE* out = render_get_sink(&sesh->renderer);
    bool had_board = sesh->has_board;
    if (op_undo(sesh) != CMD_OK) {
        fprintf(out, "nothing to undo\n");
        return CMD_ERR;
    }
    if (had_board) print_board(sesh);
    else fprintf(out, "hero hand cleared\n");
    return CMD_OK;
}

static int cmd_bomb(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    op_reset(sesh);
    op_dealbomb(sesh);

    print_hero(sesh);
    print_villain(sesh);
    print_board(sesh);

    render_blank(&sesh->renderer);
    return CMD_OK;
}

static int cmd_hero(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    op_ensure_hero(sesh);
    print_hero(sesh);
    return CMD_OK;
}

static int cmd_villain(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    op_ensure_hero(sesh);
    print_villain(sesh);
    return CMD_OK;
}

static int cmd_board(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    op_ensure_hero(sesh);
    op_ensure_board(sesh);
    print_board(sesh);
    return CMD_OK;
}

static const char* mode_name(RenderMode m) {
    switch (m) {
    case RENDER_STATE:      return "state";
    case RENDER_PURITY:     return "purity";
    case RENDER_DRAW:       return "draw";
    case RENDER_SUIT:       return "suit";
    case RENDER_FLUSH:      return "flush";
    case RENDER_EQUITY:     return "equity";
    case RENDER_JOINT:      return "joint";
    case RENDER_ENTROPY:    return "entropy";
    case RENDER_VOLATILITY: return "volatility";
    default:                return "?";
    }
}

static int cmd_render_settings(Session* sesh, int argc, char** argv) {
    FILE* out = render_get_sink(&sesh->renderer);

    if (argc == 0) {
        fprintf(out, "symset: %s  width: %d  mode: %s\n",
                sesh->renderer.symset == SYMSET_UNICODE ? "unicode" : "ascii",
                sesh->renderer.width == CELL_1 ? 1 :
                sesh->renderer.width == CELL_2 ? 2 : 4,
                mode_name(sesh->renderer.mode));
        return CMD_OK;
    }
    for (int i = 0; i < argc; i++) {
        const char* a = argv[i];
        if      (strcmp(a, "unicode")     == 0) sesh->renderer.symset = SYMSET_UNICODE;
        else if (strcmp(a, "ascii")       == 0) sesh->renderer.symset = SYMSET_ASCII;

        else if (strcmp(a, "1")           == 0) sesh->renderer.width  = CELL_1;
        else if (strcmp(a, "2")           == 0) sesh->renderer.width  = CELL_2;
        else if (strcmp(a, "4")           == 0) sesh->renderer.width  = CELL_4;

        else if (strcmp(a, "state")       == 0) sesh->renderer.mode   = RENDER_STATE;
        else if (strcmp(a, "purity")      == 0) sesh->renderer.mode   = RENDER_PURITY;
        else if (strcmp(a, "draw")        == 0) sesh->renderer.mode   = RENDER_DRAW;
        else if (strcmp(a, "suit")        == 0) sesh->renderer.mode   = RENDER_SUIT;
        else if (strcmp(a, "flush")       == 0) sesh->renderer.mode   = RENDER_FLUSH;
        else if (strcmp(a, "equity")      == 0) sesh->renderer.mode   = RENDER_EQUITY;
        else if (strcmp(a, "joint")       == 0) sesh->renderer.mode   = RENDER_JOINT;
        else fprintf(out, "unknown setting '%s'  (unicode|ascii | 1|2|4 | state|purity|draw|suit|flush|equity|joint)\n", a);
    }
    return CMD_OK;
}

static int cmd_analyze(Session* sesh, int argc, char** argv) {
    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    bool dashboard = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) { dashboard = true; break; }
    }

    Context ctx = get_context(sesh);
    TextPanel* window = dashboard ? make_dashboard(sesh, ctx) : make_layout(sesh, ctx);

    panel_print(window, &sesh->renderer);
    panel_free(window);

    return CMD_OK;
}

static int cmd_equity(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    Context ctx = get_context(sesh);
    TextPanel* window = make_layout(sesh, ctx);

    panel_print(window, &sesh->renderer);
    panel_free(window);

    return CMD_OK;
}

static int cmd_reset(Session* sesh, int argc, char** argv) {
	(void)argc; (void)argv;
	op_reset(sesh);
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
    { "bomb",    'B', "bomb",                        "deal bomb pot (players and flop)",                      cmd_bomb           },

	{ "print hero",    'h', "print hero",    "print hero hand",      cmd_hero    },
	{ "print villain", 'V', "print villain", "print villain hand",   cmd_villain },
	{ "print board",   'b', "print board",   "print cards on board", cmd_board   },

	{ "render",  'R', "render [unicode|ascii|1|2|4|state|purity|draw|suit|flush|equity|joint]", "get/set renderer (symset, width, mode)", cmd_render_settings},
	{ "analyze",  'a', "analyze",                      "build RangeField vs hero + board",                      cmd_analyze        },
	{ "equity",   'e', "equity",                        "build equity ScalarField vs hero + board",               cmd_equity         },
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
