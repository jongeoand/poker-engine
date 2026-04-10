static void count_combo_cb(Combo c, void* ctx) {
	(void)c;
	(*(int*)ctx)++;
}

void test_combostate(void) {
	printf("=== ComboState & HtrComboStream ===\n\n");

	// ---- combostate_str / combostate_symbol ----
	printf("State strings/symbols:\n");
	for (int i = 0; i < COMBO_STATE_COUNT; i++) {
		printf("  %s (%c)\n", combostate_str((ComboState)i), combostate_symbol((ComboState)i));
	}
	printf("\n");

	// ---- HtrComboStream: empty range produces 0 combos ----
	{
		HandTypeRange empty = htr_empty();
		HtrComboStream s;
		Combo c;
		combostream_init(&s, &empty, 0);
		int n = 0;
		while (combostream_next(&s, &c)) n++;
		printf("Stream empty HTR:      %4d combos  (expected    0) %s\n",
		       n, n == 0 ? "OK" : "FAIL");
	}

	// ---- HtrComboStream: full range, no dead cards ----
	{
		HandTypeRange full = htr_full();
		HtrComboStream s;
		Combo c;
		combostream_init(&s, &full, 0);
		int n = 0;
		while (combostream_next(&s, &c)) n++;
		printf("Stream full HTR:       %4d combos  (expected 1326) %s\n",
		       n, n == 1326 ? "OK" : "FAIL");
	}

	// ---- stream_foreach_combo: count matches direct stream ----
	{
		HandTypeRange full = htr_full();
		int n = 0;
		stream_foreach_combo(&full, 0, count_combo_cb, &n);
		printf("foreach full HTR:      %4d combos  (expected 1326) %s\n",
		       n, n == 1326 ? "OK" : "FAIL");
	}

	// ---- HtrComboStream: one dead card removes 51 combos ----
	{
		Card ac = { .suit = CLUB, .rank = ACE };
		uint64_t dead = toBitmask(ac);
		HandTypeRange full = htr_full();
		HtrComboStream s;
		Combo c;
		combostream_init(&s, &full, dead);
		int n = 0;
		while (combostream_next(&s, &c)) n++;
		printf("Stream full, Ac dead:  %4d combos  (expected 1275) %s\n\n",
		       n, n == 1275 ? "OK" : "FAIL");
	}

	// ---- classify_combostate: villain flush draw, behind hero ----
	// Board: 2h 3h 4c (flop, two hearts)
	// Hero:  Ah As  (overpair)
	// Villain: Kh Th (behind, but has flush draw with board) → COMBO_BEHIND_LIVE
	{
		uint64_t board   = toBitmask((Card){ .suit = HEART,   .rank = TWO   })
		                 | toBitmask((Card){ .suit = HEART,   .rank = THREE  })
		                 | toBitmask((Card){ .suit = CLUB,    .rank = FOUR   });
		uint64_t hero    = toBitmask((Card){ .suit = HEART,   .rank = ACE   })
		                 | toBitmask((Card){ .suit = SPADE,   .rank = ACE   });
		uint64_t villain = toBitmask((Card){ .suit = HEART,   .rank = KING  })
		                 | toBitmask((Card){ .suit = HEART,   .rank = TEN   });
		ComboState cs = classify_combostate(board, hero, villain);
		printf("KhTh vs AhAs on 2h3h4c:    %s (%c)  (expected behind_live) %s\n",
		       combostate_str(cs), combostate_symbol(cs),
		       cs == COMBO_BEHIND_LIVE ? "OK" : "FAIL");
	}

	// ---- classify_combostate: villain behind, no draws ----
	// Board: 2h 3d 4c (rainbow flop)
	// Hero:  Ah As  (overpair)
	// Villain: Ks Kd (underpair, rainbow board = no flush draw, no straight draws for KK) → COMBO_BEHIND_DEAD
	{
		uint64_t board   = toBitmask((Card){ .suit = HEART,   .rank = TWO   })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = THREE  })
		                 | toBitmask((Card){ .suit = CLUB,    .rank = FOUR   });
		uint64_t hero    = toBitmask((Card){ .suit = HEART,   .rank = ACE   })
		                 | toBitmask((Card){ .suit = SPADE,   .rank = ACE   });
		uint64_t villain = toBitmask((Card){ .suit = SPADE,   .rank = KING  })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = KING  });
		ComboState cs = classify_combostate(board, hero, villain);
		printf("KsKd vs AhAs on 2h3d4c:    %s (%c)  (expected behind_dead) %s\n",
		       combostate_str(cs), combostate_symbol(cs),
		       cs == COMBO_BEHIND_DEAD ? "OK" : "FAIL");
	}

	// ---- classify_combostate: villain ahead ----
	// Board: 2h 3d 4c Kh (turn)
	// Hero:  2c 2d (trips 2s)
	// Villain: Kc Kd (trips Ks, higher trips) → COMBO_AHEAD
	{
		uint64_t board   = toBitmask((Card){ .suit = HEART,   .rank = TWO   })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = THREE  })
		                 | toBitmask((Card){ .suit = CLUB,    .rank = FOUR   })
		                 | toBitmask((Card){ .suit = HEART,   .rank = KING  });
		uint64_t hero    = toBitmask((Card){ .suit = CLUB,    .rank = TWO   })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = TWO   });
		uint64_t villain = toBitmask((Card){ .suit = CLUB,    .rank = KING  })
		                 | toBitmask((Card){ .suit = DIAMOND, .rank = KING  });
		ComboState cs = classify_combostate(board, hero, villain);
		printf("KcKd vs 2c2d on 2h3d4cKh:  %s (%c)  (expected ahead) %s\n\n",
		       combostate_str(cs), combostate_symbol(cs),
		       cs == COMBO_AHEAD ? "OK" : "FAIL");
	}

	// ---- count_combostates: buckets sum to total, total matches stream ----
	// Board: Ah Kd Qc (flop)
	// Hero:  Jh Td (open-ended straight draw)
	// Dead:  board | hero (5 blocked cards)
	{
		uint64_t board = toBitmask((Card){ .suit = HEART,   .rank = ACE   })
		               | toBitmask((Card){ .suit = DIAMOND, .rank = KING  })
		               | toBitmask((Card){ .suit = CLUB,    .rank = QUEEN });
		uint64_t hero  = toBitmask((Card){ .suit = HEART,   .rank = JACK  })
		               | toBitmask((Card){ .suit = DIAMOND, .rank = TEN   });
		uint64_t dead  = board | hero;

		HandTypeRange full = htr_full();
		ComboStateCounts csc = count_combostates(&full, board, hero, dead);

		HtrComboStream stream;
		Combo c;
		combostream_init(&stream, &full, dead);
		int stream_n = 0;
		while (combostream_next(&stream, &c)) stream_n++;

		int sum = 0;
		printf("count_combostates(full, AhKdQc, JhTd):\n");
		for (int i = 0; i < COMBO_STATE_COUNT; i++) {
			printf("  %-12s: %d\n", combostate_str((ComboState)i), csc.counts[i]);
			sum += csc.counts[i];
		}
		printf("  total:           %d\n", csc.total);
		printf("  stream count:    %d\n", stream_n);
		printf("  buckets sum == total:  %s\n", sum == csc.total ? "OK" : "FAIL");
		printf("  total == stream:       %s\n\n", csc.total == stream_n ? "OK" : "FAIL");
	}
}
