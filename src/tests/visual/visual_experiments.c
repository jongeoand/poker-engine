void visual_experiments(void) {
    Renderer r = render_default();
    r.symset = SYMSET_UNICODE;
    r.width  = CELL_2;

    /* --- Experiment 1: single game, flop / turn / river --- */
    {
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
    }

    /* --- Experiment 2: all hero combos against a fixed board --- */
    {
        render_heading(&r, "Experiment 2");

        
        Game game = make_game(2);
        deal_players(&game);
        deal_street(&game); uint64_t flop  = game.board;
        deal_street(&game); uint64_t turn  = game.board;
        deal_street(&game); uint64_t river = game.board;

        Combo    villain     = game.playerhands[1];
        uint64_t villainmask = toBitmask(villain.a) | toBitmask(villain.b);
        uint64_t boards[3]   = { flop, turn, river };

        HandTypeRange full = htr_full();

        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                Combo    c        = { make_card(i), make_card(j) };
                uint64_t heromask = toBitmask(c.a) | toBitmask(c.b);

                if (heromask & (villainmask | river)) continue;

                TextPanel* view = NULL;

                for (int k = 0; k < 3; k++) {
                    uint64_t board = boards[k];
                    RangeField rf_hero    = hmap_build(&full, heromask    | board, board, heromask);
                    RangeField rf_villain = hmap_build(&full, villainmask | board, board, villainmask);

                    TextPanel* p_hero    = views_rangefield(&r, &rf_hero);
                    TextPanel* p_villain = views_rangefield(&r, &rf_villain);
                    TextPanel* stacked   = panel_stack_consume(p_hero, p_villain);

                    view = (view == NULL) ? stacked : panel_join_consume(view, stacked, 2);
                }

                panel_print(view, &r);
                panel_free(view);
            }
        }
    }

    /* --- Experiment 3: 25 random games, flop / turn / river --- */
    {
        render_heading(&r, "Experiment 3");
        HandTypeRange full = htr_full();

        for (int k = 0; k < 25; k++) {
            Game game = make_game(2);
            TextPanel* view = NULL;

            deal_players(&game);

            Combo    hero        = game.playerhands[0];
            Combo    villain     = game.playerhands[1];
            uint64_t heromask    = toBitmask(hero.a)    | toBitmask(hero.b);
            uint64_t villainmask = toBitmask(villain.a) | toBitmask(villain.b);
            
            for (int i = 0; i < 3; i++) {
                if (i == 0) deal_street(&game);

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
        }
    }
}
