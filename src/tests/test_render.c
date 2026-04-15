void test_render(void) {
    // Random board and hero hand each time
    Game game = make_game(6);
    deal_players(&game);
    deal_street(&game);

    uint64_t board = game.board;
    Combo holecards = game.playerhands[0];

    uint64_t hero = combo_toBitmask(holecards);
    uint64_t dead = board | hero;

    HandTypeRange full = htr_full();

    RangeField rdata = hmap_build(&full, dead, board, hero);

    Renderer write = render_default();

#define VPRINT(panel_expr, rend) do { TextPanel* _vp = (panel_expr); panel_print(_vp, (rend)); panel_free(_vp); } while(0)

    render_heading(&write, "test render heading");

    VPRINT(views_htr_grid(&full), &write); printf("\n");

    render_heading(&write, "default renderer");

    VPRINT(views_rangefield(&write, &rdata, NULL), &write);

	printf("\n");
    render_heading(&write, "symset unicode");
    write.symset = SYMSET_UNICODE;

    VPRINT(views_rangefield(&write, &rdata, NULL), &write);
    render_blank(&write);
    write.symset = SYMSET_ASCII;
    write.mode = RENDER_PURITY;

    render_heading(&write, "render mode: purity");

    VPRINT(views_rangefield(&write, &rdata, NULL), &write);
    render_blank(&write);

    write.symset = SYMSET_UNICODE;
    VPRINT(views_rangefield(&write, &rdata, NULL), &write);

    render_blank(&write);

    render_heading(&write, "render mode: draw");
    write.symset = SYMSET_ASCII;
    write.mode = RENDER_DRAW;

    VPRINT(views_rangefield(&write, &rdata, NULL), &write);
    render_blank(&write);

    write.symset = SYMSET_UNICODE;
    VPRINT(views_rangefield(&write, &rdata, NULL), &write);

#undef VPRINT
}
