// Board and hero shared across tests 1–4:
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
}
