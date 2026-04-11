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
    StateField sdata; hmap_project_state(&rdata, & sdata);

    Renderer write = render_default();

    render_heading(&write, "test render heading");
    
    views_htr_grid(&write, &full); printf("\n");

    render_heading(&write, "default renderer");
    
    views_rangefield(&write, &rdata);
	
	printf("\n");
    views_statefield(&write, &sdata);  

    render_heading(&write, "symset unicode");
    write.symset = SYMSET_UNICODE;
    
    views_rangefield(&write, &rdata);
    render_blank(&write);
    views_statefield(&write, &sdata);

    write.symset = SYMSET_ASCII;
    write.mode = RENDER_PURITY;

    render_heading(&write, "render mode: purity");
    
    views_rangefield(&write, &rdata);
    render_blank(&write);

    write.symset = SYMSET_UNICODE;
    views_rangefield(&write, &rdata);
    
    render_blank(&write);

    render_heading(&write, "render mode: draw");
    write.symset = SYMSET_ASCII;
    write.mode = RENDER_DRAW;

    views_rangefield(&write, &rdata);
    render_blank(&write);

    write.symset = SYMSET_UNICODE;
    views_rangefield(&write, &rdata);
}
