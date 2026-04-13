void visual_test(void) {
    Renderer r = render_default();
    r.symset = SYMSET_UNICODE;
    r.width  = CELL_2;

    Game game = make_game(2);
    deal_players(&game);
    deal_street(&game);

    Combo    hero        = game.playerhands[0];
    Combo    villain     = game.playerhands[1];
    uint64_t heromask    = toBitmask(hero.a)    | toBitmask(hero.b);
    uint64_t villainmask = toBitmask(villain.a) | toBitmask(villain.b);

    HandTypeRange full = htr_full();

    /* Flop — hero POV stacked above villain POV */
    RangeField rf_hero_flop    = hmap_build(&full, heromask    | game.board, game.board, heromask);
    RangeField rf_villain_flop = hmap_build(&full, villainmask | game.board, game.board, villainmask);

    TextPanel* p_hero_flop    = views_rangefield(&r, &rf_hero_flop);
    TextPanel* p_villain_flop = views_rangefield(&r, &rf_villain_flop);
    TextPanel* stacked_flop   = panel_stack_consume(p_hero_flop, p_villain_flop);
    
    /* Turn — same layout */
    deal_street(&game);

    RangeField rf_hero_turn    = hmap_build(&full, heromask    | game.board, game.board, heromask);
    RangeField rf_villain_turn = hmap_build(&full, villainmask | game.board, game.board, villainmask);

    TextPanel* p_hero_turn    = views_rangefield(&r, &rf_hero_turn);
    TextPanel* p_villain_turn = views_rangefield(&r, &rf_villain_turn);
    TextPanel* stacked_turn   = panel_stack_consume(p_hero_turn, p_villain_turn);

    /* Join flop | turn side-by-side and display */
    TextPanel* joined = panel_join_consume(stacked_flop, stacked_turn, 2);
 
    RangeField rf_hero_river    = hmap_build(&full, heromask    | game.board, game.board, heromask);
    RangeField rf_villain_river = hmap_build(&full, villainmask | game.board, game.board, villainmask);

    TextPanel* p_hero_river    = views_rangefield(&r, &rf_hero_river);
    TextPanel* p_villain_river = views_rangefield(&r, &rf_villain_river);
    
    TextPanel* stacked_river = panel_stack_consume(p_hero_river, p_villain_river);
    TextPanel* final = panel_join_consume(joined, stacked_river, 2);

    panel_print(final, &r);
    panel_free(final);

}
