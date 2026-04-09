#include "tests.h"

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

	printf("  -- analysis layer --\n");
	printf("  HandFeatures:   %2zu bytes\n", sizeof(HandFeatures));
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

void test_range(void) {
	printf("=== Range ===\n\n");

	Range empty = range_empty();
	printf("Empty range: %d combos\n", range_count(&empty));

	Range full = range_full();
	printf("Full range:  %d combos\n\n", range_count(&full));
	
	Range r = empty;

	printf("Building range via nested loop (combo count per card added):\n\n");
	for (int i = 0; i < 52; i++) {
		for (int j = i + 1; j < 52; j++) {
			Combo c = { make_card(i), make_card(j) };
			range_add(&r, c);
		}
		Card card = make_card(i);
		printf("  after ");
		output_card(card);
		printf(": %4d combos\n", range_count(&r));
	}

	printf("\nFull range (%d combos):\n\n", range_count(&r));
	output_range(&r);
}

void test_handtype_range(void) {
	printf("=== HandTypeRange ===\n\n");

	HandTypeRange empty = htr_empty();
	HandTypeRange full  = htr_full();

	// ---- Basic counts ----
	printf("Empty HTR: %d types\n", htr_count(&empty));
	printf("Full HTR:  %d types\n", htr_count(&full));
	printf("  Pairs:   %d\n",   htr_count_pairs(&full));
	printf("  Suited:  %d\n",   htr_count_suited(&full));
	printf("  Offsuit: %d\n\n", htr_count_offsuit(&full));

	// ---- Combo count ----
	int max_combos   = htr_combo_count_max(&full);
	int exact_combos = htr_combo_count_exact(&full, 0);
	printf("Max combos (no dead):   %d\n", max_combos);
	printf("Exact combos (no dead): %d\n\n", exact_combos);

	// ---- Index roundtrip ----
	bool ok = true;
	for (int i = 0; i < HANDTYPE_COUNT; i++) {
		if (handtype_index(handtype_from_index(i)) != i) { ok = false; break; }
	}
	printf("Index roundtrip (0..168): %s\n\n", ok ? "OK" : "FAIL");

	// ---- Materialize ----
	Range mat = htr_materialize(&full, 0);
	printf("htr_materialize(full, dead=0): %d combos\n", range_count(&mat));

	// ---- Compress ----
	Range rf = range_full();
	HandTypeRange comp = range_compress(&rf);
	printf("range_compress(range_full()): %d types (%s)\n\n",
		htr_count(&comp),
		htr_count(&comp) == HANDTYPE_COUNT ? "OK" : "FAIL");

	// ---- Set operations ----
	HandTypeRange pairs_only = htr_empty();
	for (int r = 0; r < 13; r++) htr_add(&pairs_only, make_pair((uint8_t)r));

	HandTypeRange no_pairs = htr_subtract(&full, &pairs_only);
	printf("Set ops:\n");
	printf("  pairs_only count: %d\n",   htr_count(&pairs_only));
	printf("  full - pairs:     %d\n",   htr_count(&no_pairs));
	HandTypeRange inter = htr_intersect(&pairs_only, &no_pairs);
	printf("  intersect:        %d\n\n", htr_count(&inter));
}

void test(void) {
	Game game = make_game(6);
	deal_players(&game);
		
	HandTypeRange range = htr_full();
	output_htr(&range); printf("\n");
	
	Combo hero = game.playerhands[0];
	printf("Hero: "); output_combo(hero);
	printf("----------------------------\n");

	for (int s = 0; s < 3; s++) {
		deal_street(&game);
		output_board(game.board); printf("\n");	
		printf("Hero: "); output_combo(hero); printf("\n\n\n");
		
		HandTypeRange value = behind(&game, 0);
		HandTypeRange bluffs = aheadof(&game, 0);
		
		printf("Hands Hero Beats: \n");
		output_htr(&bluffs); printf("\n\n");
		
		printf("OESD / Gutshots: \n");
		HandTypeRange straight = straight_draws(&game, 0);
		output_htr(&straight); printf("\n\n");
		
		if (s == 0) {
			printf("Backdoor straight draws: \n");
			HandTypeRange backdoors = backdoor_straights(&game, 0);
			output_htr(&backdoors); printf("\n\n");
		} 

		printf("Hands better than Hero: \n");
		output_htr(&value); printf("\n");
	}
} 
