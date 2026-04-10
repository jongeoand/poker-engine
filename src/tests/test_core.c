void print_struct_sizes(void) {
	printf("=== Struct Sizes ===\n\n");

	printf("  -- primitives --\n");
	printf("  Card:           %2zu bytes\n", sizeof(Card));
	printf("  Combo:          %2zu bytes\n", sizeof(Combo));
	printf("  HandType:       %2zu bytes\n", sizeof(HandType));
	printf("  DrawInfo:       %2zu bytes\n", sizeof(DrawInfo));
	printf("  Outs:           %2zu bytes\n", sizeof(Outs));
	printf("  Range:          %2zu bytes\n", sizeof(Range));
	printf("  HandTypeRange:  %2zu bytes\n", sizeof(HandTypeRange));
	printf("\n");

	printf("  -- range iteration --\n");
	printf("  HtrComboStream: %2zu bytes\n", sizeof(HtrComboStream));
	printf("\n");

	printf("  -- analysis layer --\n");
	printf("  HandFeatures:   %2zu bytes\n", sizeof(HandFeatures));
	printf("  ComboStateCounts:%2zu bytes\n", sizeof(ComboStateCounts));
	printf("\n");

	printf("  -- htrange profiles --\n");
	printf("  HtrBoardProfile:%2zu bytes\n", sizeof(HtrBoardProfile));
	printf("  HtrEquitySplit: %2zu bytes\n", sizeof(HtrEquitySplit));
	printf("\n");
}

void test_cards(void) {
	printf("=== Cards ===\n\n");

	printf("Enumeration via nested loop (suit × rank):\n\n");
	for (int s = 0; s < 4; s++) {
		for (int r = 0; r < 13; r++) {
			Card card = { .suit = s, .rank = r };
			output_card(card);
			printf("  ");
		}
		printf("\n");
	}

	printf("\nEnumeration via make_card(i):\n\n");
	for (int i = 0; i < 52; i++) {
		if (i % 13 == 0 && i > 0) printf("\n");
		output_card(make_card(i));
		printf("  ");
	}
	printf("\n\n");

	printf("Bitmask for Ac:\n");
	Card ac = { .suit = CLUB, .rank = ACE };
	output_binary(toBitmask(ac));
	printf("\n");
}

void test_combos(void) {
	printf("=== Combos ===\n\n");

	printf("Enumeration via nested loop (i < j over 52 cards):\n\n");
	int count = 0;
	for (int i = 0; i < 52; i++) {
		for (int j = i + 1; j < 52; j++) {
			Combo c = { make_card(i), make_card(j) };
			output_combo(c);
			printf(" ");
			if (++count % 13 == 0) printf("\n");
		}
	}
	printf("\nTotal: %d combos\n\n", count);

	printf("Enumeration via combo_from_index(i):\n\n");
	for (int i = 0; i < COMBO_COUNT; i++) {
		Combo c = combo_from_index(i);
		output_combo(c);
		printf(" ");
		if ((i + 1) % 13 == 0) printf("\n");
	}
	printf("\n\n");

	printf("Combo properties (first 10):\n\n");
	for (int i = 0; i < 10; i++) {
		Combo c = combo_from_index(i);
		output_combo(c);
		printf("  suited=%-2d  pair=%-2d  hi=%c  lo=%c  idx=%d\n",
			combo_is_suited(c),
			combo_is_pair(c),
			rank_to_char(combo_high_rank(c)),
			rank_to_char(combo_low_rank(c)),
			combo_index(c));
	}
	printf("\n");
}

void test_handtypes(void) {
	printf("=== Hand Types ===\n\n");

	printf("Pairs via make_pair (13):\n\n");
	for (int r = 12; r >= 0; r--) {
		HandType ht = make_pair((uint8_t)r);
		output_handtype(ht);
		printf("  %d combos\n", handtype_combo_count(ht));
	}

	printf("\nSuited via make_suited (78):\n\n");
	int col = 0;
	for (int h = 12; h >= 1; h--) {
		for (int l = h - 1; l >= 0; l--) {
			HandType ht = make_suited((uint8_t)h, (uint8_t)l);
			output_handtype(ht);
			printf(" ");
			if (++col % 13 == 0) printf("\n");
		}
	}

	printf("\n\nOffsuit via make_offsuit (78):\n\n");
	col = 0;
	for (int h = 12; h >= 1; h--) {
		for (int l = h - 1; l >= 0; l--) {
			HandType ht = make_offsuit((uint8_t)h, (uint8_t)l);
			output_handtype(ht);
			printf(" ");
			if (++col % 13 == 0) printf("\n");
		}
	}

	printf("\n\nHandType via combo_to_handtype (first 10 combos):\n\n");
	for (int i = 0; i < 10; i++) {
		Combo c = combo_from_index(i);
		HandType ht = combo_to_handtype(c);
		output_combo(c);
		printf("  -> ");
		output_handtype(ht);
		printf("  combos=%d\n", handtype_combo_count(ht));
	}

	printf("\nTotal: %d hand types\n\n", HANDTYPE_COUNT);
}
