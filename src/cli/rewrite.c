// Session lifecycle

Session session_default(void) {
    Session s;
    s.game = make_game(2);
    s.renderer = render_default();

    bool has_hero = false;
    bool has_board = false;
} Session;

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
    
    sesh->has_hero = true;

    print_hero(sesh);
    return CMD_OK;
}

static int cmd_street(Session* sesh, int argc, char** argv) {
    (void)argc; (void)argv;
    
    deal_street(&sesh->game);
    
    print_board(sesh);
    return CMD_OK
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
    
    if (sesh->has_board) {print_board(sesh); return CMD_OK};

    deal_street(&sesh->game);
    sesh->has_board = true;
    print_board(sesh);

    return CMD_OK;
}


