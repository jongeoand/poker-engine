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

    // Experiment 4 — villain combo stream, flop / turn / river
    /*
    {
        r = render_default();
        render_heading(&r, "Experiment 4");

        Game g = make_game(2);
        deal_bomb(&g);          uint64_t flop  = g.board;
        deal_street(&g);        uint64_t turn  = g.board;
        deal_street(&g);        uint64_t river = g.board;

        uint64_t boards[3] = { flop, turn, river };

        Combo    hero        = g.playerhands[0];
        uint64_t hmask       = combo_toBitmask(hero);
        uint64_t stream_dead = hmask | river;

        Renderer r4 = render_default();
        r4.symset = SYMSET_UNICODE;
        r4.mode   = RENDER_EQUITY;
        r4.width  = CELL_2;

        HandTypeRange full = htr_full();

        HtrComboStream stream;
        combostream_init(&stream, &full, stream_dead);

        Combo current;
        while (combostream_next(&stream, &current)) {
            uint64_t vmask = combo_toBitmask(current);
            uint64_t vdead = vmask | river;

            TextPanel* layout = NULL;

            for (int s = 0; s < 3; s++) {
                uint64_t    board = boards[s];
                RangeField  rf    = hmap_build(&full, vdead, board, vmask);
                ScalarField sf    = scalar_build(&rf, vdead, board, vmask);
                TextPanel*  p     = views_rangefield(&r4, &rf, &sf);
                layout = (layout == NULL) ? p : panel_join_consume(layout, p, 2);
            }

            render_combo(&r, current);
            render_heading(&r, " ");
            panel_print(layout, &r4);
            panel_free(layout);
            render_blank(&r4);
        }
    } */

    // Experiment 5 — full runout stream stress test
    // Fixed flop. For each live villain combo, iterate every live turn card;
    // for each turn, iterate every live river card and print joined panels.
    //
    // Fun to play with, but not to eat. Took 1282.15s (21 minutes) to run on my machine 
    /*{
        r = render_default();
        render_heading(&r, "Experiment 5");

        Game g = make_game(2);
        deal_bomb(&g);

        uint64_t flop  = g.board;
        Combo    hero  = g.playerhands[0];
        uint64_t hmask = combo_toBitmask(hero);

        Renderer r5 = render_default();
        r5.symset = SYMSET_UNICODE;
        r5.mode   = RENDER_EQUITY;
        r5.width  = CELL_2;

        HandTypeRange full = htr_full();

        HtrComboStream stream;
        combostream_init(&stream, &full, hmask | flop);

        Combo current;
        while (combostream_next(&stream, &current)) {
            uint64_t vmask     = combo_toBitmask(current);
            uint64_t dead_base = vmask | flop;

            // Flop panel — fixed for all runouts of this villain combo 
            RangeField  rf_flop = hmap_build(&full, dead_base, flop, vmask);
            ScalarField sf_flop = scalar_build(&rf_flop, dead_base, flop, vmask);
            TextPanel*  p_flop  = views_rangefield(&r5, &rf_flop, &sf_flop);

            for (int t = 0; t < 52; t++) {
                uint64_t t_bit = 1ULL << t;
                if (t_bit & dead_base) continue;

                uint64_t turn_board = flop | t_bit;
                uint64_t dead_turn  = dead_base | t_bit;

                // Turn panel — fixed for all rivers on this turn 
                RangeField  rf_turn = hmap_build(&full, dead_turn, turn_board, vmask);
                ScalarField sf_turn = scalar_build(&rf_turn, dead_turn, turn_board, vmask);
                TextPanel*  p_turn  = views_rangefield(&r5, &rf_turn, &sf_turn);

                for (int rv = 0; rv < 52; rv++) {
                    uint64_t rv_bit = 1ULL << rv;
                    if (rv_bit & dead_turn) continue;

                    uint64_t river_board = turn_board | rv_bit;
                    uint64_t dead_river  = dead_turn  | rv_bit;

                    RangeField  rf_river = hmap_build(&full, dead_river, river_board, vmask);
                    ScalarField sf_river = scalar_build(&rf_river, dead_river, river_board, vmask);
                    TextPanel*  p_river  = views_rangefield(&r5, &rf_river, &sf_river);

                    TextPanel* ft  = panel_join(p_flop, p_turn, 2);
                    TextPanel* ftr = panel_join_consume(ft, p_river, 2);
                    render_combo(&r, current);
                    render_board(&r, river_board);
                    render_heading(&r, " ");
                    panel_print(ftr, &r5);
                    panel_free(ftr);
                }

                panel_free(p_turn);
            }

            panel_free(p_flop);
            render_blank(&r5);
        }
    }*/
}
