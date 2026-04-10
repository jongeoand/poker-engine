// Board and hero shared across tests 1–6:
//   Board: 2h 3d 4c  (no aces or kings — won't block AKs or TT)
//   Hero:  5s 6s     (completes a 2-3-4-5-6 straight — beats all AKs and TT)
//
// AKs states on this board/hero (dead=0):
//   AhKh  3 hearts (Ah Kh 2h)       → flush draw  → COMBO_BEHIND_LIVE
//   AcKc  3 clubs  (Ac Kc 4c)       → flush draw  → COMBO_BEHIND_LIVE
//   AdKd  3 diamonds (Ad Kd 3d)     → flush draw  → COMBO_BEHIND_LIVE
//   AsKs  no flush draw, gutshot A-5 → COMBO_BEHIND_LIVE
//
// TT states on this board/hero (dead=0):
//   All 6 combos: pair tens < hero straight, no draws → COMBO_BEHIND_DEAD

void test_handmap(void) {
	printf("=== RangeField / HMapCell ===\n\n");

	uint64_t board = toBitmask((Card){ .suit = HEART,   .rank = TWO   })
	               | toBitmask((Card){ .suit = DIAMOND, .rank = THREE  })
	               | toBitmask((Card){ .suit = CLUB,    .rank = FOUR   });
	uint64_t hero  = toBitmask((Card){ .suit = SPADE,   .rank = FIVE   })
	               | toBitmask((Card){ .suit = SPADE,   .rank = SIX    });

	// ---- Test 1: AKs only, dead=0 ----
	// Expect: only cell (0,1) populated, combo_total=4, all BEHIND_LIVE
	{
		HandTypeRange htr = htr_empty();
		htr_add(&htr, make_suited((uint8_t)ACE, (uint8_t)KING));

		RangeField topo = hmap_build(&htr, 0, board, hero);

		int populated = 0, pop_row = -1, pop_col = -1;
		for (int r = 0; r < HMAP_DIM; r++) {
			for (int c = 0; c < HMAP_DIM; c++) {
				if (!hmap_cell_isempty(&topo.grid[r][c])) {
					populated++;
					pop_row = r; pop_col = c;
				}
			}
		}

		int total = hmap_total(&topo);
		int behind_live = topo.grid[0][1].statecounts[COMBO_BEHIND_LIVE];

		printf("Test 1 - AKs, dead=0:\n");
		printf("  populated cells: %d  (expected 1) %s\n", populated, populated == 1 ? "OK" : "FAIL");
		printf("  cell location:   (%d,%d)  (expected (0,1)) %s\n", pop_row, pop_col, (pop_row==0 && pop_col==1) ? "OK" : "FAIL");
		printf("  combo_total:     %d  (expected 4) %s\n", total, total == 4 ? "OK" : "FAIL");
		printf("  behind_live:     %d  (expected 4) %s\n\n", behind_live, behind_live == 4 ? "OK" : "FAIL");
	}

	// ---- Test 2: AKs only, Ac dead ----
	// Expect: only (0,1) populated, combo_total=3 (AcKc blocked)
	{
		uint64_t dead = toBitmask((Card){ .suit = CLUB, .rank = ACE });

		HandTypeRange htr = htr_empty();
		htr_add(&htr, make_suited((uint8_t)ACE, (uint8_t)KING));

		RangeField topo = hmap_build(&htr, dead, board, hero);

		int total      = hmap_total(&topo);
		int behind_live = topo.grid[0][1].statecounts[COMBO_BEHIND_LIVE];
		bool only_01   = !hmap_cell_isempty(&topo.grid[0][1]);

		printf("Test 2 - AKs, Ac dead:\n");
		printf("  combo_total:     %d  (expected 3) %s\n", total, total == 3 ? "OK" : "FAIL");
		printf("  behind_live:     %d  (expected 3) %s\n", behind_live, behind_live == 3 ? "OK" : "FAIL");
		printf("  only (0,1):      %s\n\n", only_01 ? "OK" : "FAIL");
	}

	// ---- Test 3: TT only, dead=0 ----
	// Expect: only cell (4,4) populated, combo_total=6, all BEHIND_DEAD
	{
		HandTypeRange htr = htr_empty();
		htr_add(&htr, make_pair((uint8_t)TEN));

		RangeField topo = hmap_build(&htr, 0, board, hero);

		int populated = 0, pop_row = -1, pop_col = -1;
		for (int r = 0; r < HMAP_DIM; r++) {
			for (int c = 0; c < HMAP_DIM; c++) {
				if (!hmap_cell_isempty(&topo.grid[r][c])) {
					populated++;
					pop_row = r; pop_col = c;
				}
			}
		}

		int total      = hmap_total(&topo);
		int behind_dead = topo.grid[4][4].statecounts[COMBO_BEHIND_DEAD];

		printf("Test 3 - TT, dead=0:\n");
		printf("  populated cells: %d  (expected 1) %s\n", populated, populated == 1 ? "OK" : "FAIL");
		printf("  cell location:   (%d,%d)  (expected (4,4)) %s\n", pop_row, pop_col, (pop_row==4 && pop_col==4) ? "OK" : "FAIL");
		printf("  combo_total:     %d  (expected 6) %s\n", total, total == 6 ? "OK" : "FAIL");
		printf("  behind_dead:     %d  (expected 6) %s\n\n", behind_dead, behind_dead == 6 ? "OK" : "FAIL");
	}

	// ---- Test 4: full support, dead=0 ----
	// Expect: hmap_total == stream combo count (1326)
	{
		HandTypeRange full = htr_full();
		RangeField topo = hmap_build(&full, 0, board, hero);
		int topo_total = hmap_total(&topo);

		HtrComboStream stream;
		Combo c;
		combostream_init(&stream, &full, 0);
		int stream_n = 0;
		while (combostream_next(&stream, &c)) stream_n++;

		printf("Test 4 - full, dead=0:\n");
		printf("  topology total: %d\n", topo_total);
		printf("  stream count:   %d\n", stream_n);
		printf("  match:          %s\n\n", topo_total == stream_n ? "OK" : "FAIL");
	}

	// ---- Test 5: consistency with ComboStateCounts ----
	// Board: Ah Kd Qc  |  Hero: Jh Td  |  Dead: board | hero
	// Expect: hmap_count(s) == ComboStateCounts.counts[s] for all s
	//         hmap_total         == ComboStateCounts.total
	{
		uint64_t board5 = toBitmask((Card){ .suit = HEART,   .rank = ACE   })
		                | toBitmask((Card){ .suit = DIAMOND, .rank = KING  })
		                | toBitmask((Card){ .suit = CLUB,    .rank = QUEEN });
		uint64_t hero5  = toBitmask((Card){ .suit = HEART,   .rank = JACK  })
		                | toBitmask((Card){ .suit = DIAMOND, .rank = TEN   });
		uint64_t dead5  = board5 | hero5;

		HandTypeRange full = htr_full();
		RangeField    topo = hmap_build(&full, dead5, board5, hero5);
		ComboStateCounts csc  = count_combostates(&full, board5, hero5, dead5);

		int topo_total = hmap_total(&topo);
		bool all_ok = true;

		printf("Test 5 - consistency with ComboStateCounts (AhKdQc board, JhTd hero):\n");
		for (int i = 0; i < COMBO_STATE_COUNT; i++) {
			int ts = hmap_count(&topo, (ComboState)i);
			int cs = csc.counts[i];
			bool ok = (ts == cs);
			if (!ok) all_ok = false;
			printf("  %-12s: topology=%4d  global=%4d  %s\n",
			       combostate_str((ComboState)i), ts, cs, ok ? "OK" : "FAIL");
		}
		printf("  total:         topology=%4d  global=%4d  %s\n",
		       topo_total, csc.total, topo_total == csc.total ? "OK" : "FAIL");
		printf("  all states match: %s\n\n", all_ok ? "OK" : "FAIL");
	}

	// ---- Test 6: hmap_project_state — AKs only ----
	// All 4 AKs combos are BEHIND_LIVE → (0,1) projects to COMBO_BEHIND_LIVE.
	// All other cells are empty → sentinel COMBO_BEHIND_DEAD in StateField.
	{
		HandTypeRange htr = htr_empty();
		htr_add(&htr, make_suited((uint8_t)ACE, (uint8_t)KING));

		RangeField rf = hmap_build(&htr, 0, board, hero);
		StateField sf;
		hmap_project_state(&rf, &sf);

		bool cell_ok = (sf.grid[0][1] == COMBO_BEHIND_LIVE);

		int live_count = 0;
		for (int r = 0; r < HMAP_DIM; r++)
			for (int c = 0; c < HMAP_DIM; c++)
				if (sf.grid[r][c] == COMBO_BEHIND_LIVE)
					live_count++;

		printf("Test 6 - project_state, AKs only:\n");
		printf("  (0,1) == BEHIND_LIVE:      %s\n", cell_ok ? "OK" : "FAIL");
		printf("  live cells in StateField:  %d  (expected 1) %s\n\n",
		       live_count, live_count == 1 ? "OK" : "FAIL");
	}

	// ---- Test 7: hmap_project_state — full range spot-check ----
	// For each of 5 sampled cells, recompute the dominant state from the RangeField
	// directly and verify the projected StateField agrees.
	{
		HandTypeRange full = htr_full();
		RangeField rf = hmap_build(&full, 0, board, hero);
		StateField sf;
		hmap_project_state(&rf, &sf);

		int spots[5][2] = { {0,0}, {0,1}, {4,4}, {1,0}, {0,12} };
		bool all_ok = true;

		printf("Test 7 - project_state, full range spot-check:\n");
		for (int i = 0; i < 5; i++) {
			int r = spots[i][0], c = spots[i][1];
			const HMapCell* cell = &rf.grid[r][c];

			ComboState expected = COMBO_BEHIND_DEAD;
			if (cell->combo_total > 0) {
				expected = COMBO_AHEAD;
				for (int s = 1; s < COMBO_STATE_COUNT; s++)
					if (cell->statecounts[s] > cell->statecounts[expected])
						expected = (ComboState)s;
			}

			bool ok = (sf.grid[r][c] == expected);
			if (!ok) all_ok = false;
			printf("  (%d,%d): projected=%-12s  expected=%-12s  %s\n",
			       r, c, combostate_str(sf.grid[r][c]), combostate_str(expected),
			       ok ? "OK" : "FAIL");
		}
		printf("  all match: %s\n\n", all_ok ? "OK" : "FAIL");
	}

	// ---- Test 8: output_rangefield — line count and symbol check ----
	// AKs only, CELL_1, SYMSET_ASCII, RENDER_STATE.
	// Row 0: col 0 = AA (empty → '.'), col 1 = AKs (BEHIND_LIVE → 'L').
	// Total output: 13 lines (one per row).
	{
		HandTypeRange htr = htr_empty();
		htr_add(&htr, make_suited((uint8_t)ACE, (uint8_t)KING));
		RangeField rf = hmap_build(&htr, 0, board, hero);

		char buf[2048] = {0};
		FILE* tmp = tmpfile();
		output_set_sink(tmp);
		RenderConfig cfg = { RENDER_STATE, SYMSET_ASCII, CELL_1 };
		output_rangefield(&rf, cfg);
		fflush(tmp);
		rewind(tmp);
		fread(buf, 1, sizeof(buf) - 1, tmp);
		fclose(tmp);
		output_set_sink(stdout);

		int lines = 0;
		for (int i = 0; buf[i]; i++)
			if (buf[i] == '\n') lines++;

		// CELL_1: each cell is 2 chars (symbol + space). Col 0 at byte 0, col 1 at byte 2.
		char col0_sym = buf[0];
		char col1_sym = buf[2];

		bool lines_ok = (lines == 13);
		bool col0_ok  = (col0_sym == '.');
		bool col1_ok  = (col1_sym == 'L');

		printf("Test 8 - output_rangefield (CELL_1, ASCII, STATE):\n");
		printf("  line count:            %d  (expected 13) %s\n", lines, lines_ok ? "OK" : "FAIL");
		printf("  row 0 col 0 symbol: '%c'  (expected '.') %s\n", col0_sym, col0_ok ? "OK" : "FAIL");
		printf("  row 0 col 1 symbol: '%c'  (expected 'L') %s\n\n", col1_sym, col1_ok ? "OK" : "FAIL");
	}

	// ---- Test 9: output_statefield vs output_rangefield (RENDER_STATE) ----
	// On a full range with no dead cards all 169 cells are populated, so
	// output_statefield must produce the same result as output_rangefield RENDER_STATE.
	{
		HandTypeRange full = htr_full();
		RangeField rf = hmap_build(&full, 0, board, hero);
		StateField sf;
		hmap_project_state(&rf, &sf);

		char buf_rf[4096] = {0};
		char buf_sf[4096] = {0};
		RenderConfig cfg = { RENDER_STATE, SYMSET_ASCII, CELL_1 };

		FILE* t1 = tmpfile();
		output_set_sink(t1);
		output_rangefield(&rf, cfg);
		fflush(t1); rewind(t1);
		fread(buf_rf, 1, sizeof(buf_rf) - 1, t1);
		fclose(t1);

		FILE* t2 = tmpfile();
		output_set_sink(t2);
		output_statefield(&sf, cfg);
		fflush(t2); rewind(t2);
		fread(buf_sf, 1, sizeof(buf_sf) - 1, t2);
		fclose(t2);

		output_set_sink(stdout);

		bool match = true;
		for (int i = 0; buf_rf[i] || buf_sf[i]; i++) {
			if (buf_rf[i] != buf_sf[i]) { match = false; break; }
		}

		printf("Test 9 - output_statefield matches output_rangefield (full range, RENDER_STATE):\n");
		printf("  outputs match: %s\n\n", match ? "OK" : "FAIL");
	}

	// ---- Display: visual render of all modes ----
	// Board: Ah Kd Qc  |  Hero: Jh Td  (broadway straight)
	// Full villain range, dead = board | hero.
	// Shows the 13×13 topology under each RenderMode × SymSet × CellWidth combination.
	{
		uint64_t board_d = toBitmask((Card){ .suit = HEART,   .rank = ACE  })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = KING  })
		                 | toBitmask((Card){ .suit = CLUB,    .rank = QUEEN });
		uint64_t hero_d  = toBitmask((Card){ .suit = HEART,   .rank = JACK  })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = TEN   });
		uint64_t dead_d  = board_d | hero_d;

		HandTypeRange full = htr_full();
		RangeField rf = hmap_build(&full, dead_d, board_d, hero_d);
		StateField sf;
		hmap_project_state(&rf, &sf);

		printf("Display — board: Ah Kd Qc  hero: Jh Td\n");
		printf("(A=ahead  C=chop  L=behind_live  D=behind_dead  .=empty)\n\n");

		printf("output_rangefield  RENDER_STATE  SYMSET_ASCII  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_STATE, SYMSET_ASCII, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_STATE  SYMSET_UNICODE  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_STATE, SYMSET_UNICODE, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_PURITY  SYMSET_ASCII  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_PURITY, SYMSET_ASCII, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_PURITY  SYMSET_UNICODE  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_PURITY, SYMSET_UNICODE, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_DRAW  SYMSET_ASCII  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_DRAW, SYMSET_ASCII, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_DRAW  SYMSET_UNICODE  CELL_1:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_DRAW, SYMSET_UNICODE, CELL_1 });
		printf("\n");

		printf("output_rangefield  RENDER_STATE  SYMSET_ASCII  CELL_2:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_STATE, SYMSET_ASCII, CELL_2 });
		printf("\n");

		printf("output_rangefield  RENDER_STATE  SYMSET_ASCII  CELL_4:\n");
		output_rangefield(&rf, (RenderConfig){ RENDER_STATE, SYMSET_ASCII, CELL_4 });
		printf("\n");

		printf("output_statefield  SYMSET_ASCII:\n");
		output_statefield(&sf, (RenderConfig){ RENDER_STATE, SYMSET_ASCII, CELL_1 });
		printf("\n");

		printf("output_statefield  SYMSET_UNICODE:\n");
		output_statefield(&sf, (RenderConfig){ RENDER_STATE, SYMSET_UNICODE, CELL_1 });
		printf("\n");
	}
}
