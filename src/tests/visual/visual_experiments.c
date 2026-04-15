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

            TextPanel* p_hero    = views_rangefield(&r, &rf_hero, NULL);
            TextPanel* p_villain = views_rangefield(&r, &rf_villain, NULL);
            TextPanel* stacked   = panel_stack_consume(p_hero, p_villain, 0);

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
        
        r.width = CELL_2;
        r.mode = RENDER_JOINT;

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

                    TextPanel* p_hero    = views_rangefield(&r, &rf_hero, NULL);
                    TextPanel* p_villain = views_rangefield(&r, &rf_villain, NULL);
                    TextPanel* stacked   = panel_stack_consume(p_hero, p_villain, 0);

                    view = (view == NULL) ? stacked : panel_join_consume(view, stacked, 2);
                }

                panel_print(view, &r);
                panel_free(view);
            }
        }
    }
    
    // Experiment 3
    {   
        r = render_default();
        render_heading(&r, "Experiment 3");
        
        Game game = make_game(2);
        
        Combo hero = game.playerhands[0];
        uint64_t hmask = toBitmask(hero.a) | toBitmask(hero.b);

        Combo villain = game.playerhands[1];
        uint64_t vmask = toBitmask(villain.a) | toBitmask(villain.b);
        
        deal_street(&game); uint64_t flop  = game.board;
        deal_street(&game); uint64_t turn  = game.board;
        deal_street(&game); uint64_t river = game.board;
        uint64_t boards[3] = { flop, turn, river };
        
        Renderer r3 = render_default();
        r3.symset = SYMSET_UNICODE;
        
        HandTypeRange full = htr_full();
        
        for (int c = 0; c < 4; c++) {
            r3.width = (CellWidth) c;
            for (int i = 0; i < 7; i++) {
                 r3.mode = (RenderMode) i;
            
                TextPanel* row = NULL;

                for (int s = 0; s < 3; s++) {
                     uint64_t board = boards[s];

                    RangeField rfh = hmap_build(&full, hmask | board, board, hmask);

                    TextPanel* etching_h = views_rangefield(&r3, &rfh, NULL);

                    row = (row == NULL) ? etching_h : panel_join_consume(row, etching_h, 2);
                }

                TextPanel* legend = views_legend(&r3);
                row = panel_join_consume(row, legend, 1);
                
                panel_print(row, &r3);
                panel_free(row);
                render_blank(&r);
            }

            render_blank(&r);
            render_blank(&r);
        }
    }
}
