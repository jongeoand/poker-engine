void visual_experiments() {
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

    TextPanel* view = NULL;

    for (int i = 0; i < 3; i++) {

        RangeField rf_hero    = hmap_build(&full, heromask    | game.board, game.board, heromask);
        RangeField rf_villain = hmap_build(&full, villainmask | game.board, game.board, villainmask);

        TextPanel* p_hero    = views_rangefield(&r, &rf_hero);
        TextPanel* p_villain = views_rangefield(&r, &rf_villain);
        TextPanel* stacked   = panel_stack_consume(p_hero, p_villain);

        view = (view == NULL) ? stacked : panel_join_consume(view, stacked, 2);

        if (i < 2) deal_street(&game);
    }

    panel_print(view, &r);
    panel_free(view);

    view = NULL;

	for (int i = 0; i < 52; i++) {
		for (int j = i + 1; j < 52; j++) {
			Combo c = { make_card(i), make_card(j) };
            heromask    = toBitmask(c.a)    | toBitmask(c.b);
    
            for (int i = 0; i < 3; i++) {
                RangeField rf_hero    = hmap_build(&full, heromask    | game.board, game.board, heromask);
                RangeField rf_villain = hmap_build(&full, villainmask | game.board, game.board, villainmask);

                TextPanel* p_hero    = views_rangefield(&r, &rf_hero);
                TextPanel* p_villain = views_rangefield(&r, &rf_villain);
                TextPanel* stacked   = panel_stack_consume(p_hero, p_villain);

                view = (view == NULL) ? stacked : panel_join_consume(view, stacked, 2);

                if (i < 2) deal_street(&game);
            }

            panel_print(view, &r);
            panel_free(view);

            view = NULL;
		}
	}
}
