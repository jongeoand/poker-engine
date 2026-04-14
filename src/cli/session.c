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

TextPanel* make_rangefield_window(Context ctx, Renderer* r) {
    HandTypeRange full = htr_full();
    RangeField rangefield = hmap_build(&full, ctx.dead, ctx.board, ctx.hero_mask);
    return views_rangefield(r, &rangefield);
}

TextPanel* make_scalarfield_window(Context ctx, Renderer* r) {
    HandTypeRange full = htr_full();
    RangeField  rf = hmap_build(&full, ctx.dead, ctx.board, ctx.hero_mask);
    ScalarField sf = scalar_build(&rf, ctx.dead, ctx.board, ctx.hero_mask);
    return views_scalarfield(r, &sf);
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

static int cmd_combostream(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    Context ctx = get_context(sesh);
    HtrComboStream stream;
    HandTypeRange full = htr_full();
    combostream_init(&stream, &full, ctx.dead);

    Combo current;
    while (combostream_next(&stream, &current)) {
        op_assignvillain(sesh, current);
        
        Context ctx = get_vcontext(sesh);
        TextPanel* view = make_rangefield_window(ctx, &sesh->renderer);
        panel_print(view, &sesh->renderer);
        panel_free(view);

        render_blank(&sesh->renderer);
    }
    return CMD_OK;
}

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

static int cmd_analyze(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    Context ctx = get_context(sesh);
    TextPanel* window = make_rangefield_window(ctx, &sesh->renderer);
 
    panel_print(window, &sesh->renderer);
    panel_free(window);

    return CMD_OK;
}

static int cmd_equity(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    Context ctx = get_context(sesh);
    TextPanel* window = make_scalarfield_window(ctx, &sesh->renderer);

    panel_print(window, &sesh->renderer);
    panel_free(window);

    return CMD_OK;
}

static int cmd_layout(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;

    op_ensure_hero(sesh);
    op_ensure_board(sesh);

    TextPanel* layout = NULL;

    for (int i = 0; i < sesh->street_count; i++) {
        Context hero_ctx = get_context(sesh);
        Context villain_ctx = get_vcontext(sesh);

        TextPanel* hero_pov    = make_rangefield_window(hero_ctx, &sesh->renderer);
        TextPanel* h_equity    = make_scalarfield_window(hero_ctx, &sesh->renderer);
        TextPanel* h_joined    = panel_join_consume(hero_pov, h_equity, 2);

        TextPanel* villain_pov = make_rangefield_window(villain_ctx, &sesh->renderer);
        TextPanel* v_equity    = make_scalarfield_window(villain_ctx, &sesh->renderer);
        TextPanel* v_joined    = panel_join_consume(villain_pov, v_equity, 2);

        TextPanel* combined    = panel_stack_consume(h_joined, v_joined);
        layout = (layout == NULL) ? combined : panel_join_consume(layout, combined, 2);
    }

    panel_print(layout, &sesh->renderer);
    panel_free(layout);
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
	{ "print hero",    'h', "print hero",    "print hero hand",      cmd_hero    },
	{ "print villain", 'V', "print villain", "print villain hand",   cmd_villain },
	{ "print board",   'b', "print board",   "print cards on board", cmd_board   },
	{ "stream",  'v', "stream",                       "stream all villain combos vs hero + board",             cmd_combostream    },
	{ "render",  'R', "render [unicode|ascii|1|2|4]","toggle renderer settings",                              cmd_render_settings},
	{ "analyze",  'a', "analyze",                      "build RangeField vs hero + board",                      cmd_analyze        },
	{ "equity",   'e', "equity",                        "build equity ScalarField vs hero + board",               cmd_equity         },
	{ "layout",   'l', "layout",                       "multi-street rangefield view (hero above villain, streets side by side)", cmd_layout },
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
