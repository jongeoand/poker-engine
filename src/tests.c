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

	// ---- Board profile: Q82 rainbow ----
	uint64_t board = toBitmask((Card){SPADE, QUEEN})
	               | toBitmask((Card){DIAMOND, EIGHT})
	               | toBitmask((Card){CLUB, TWO});
	printf("Board: "); output_board(board); printf("\n");
	HtrBoardProfile prof = htr_board_profile(&full, board);
	output_htr_board_profile(&prof);

	// ---- Subset: big pairs (AA-TT) ----
	printf("\nBig pairs (AA-TT):\n");
	HandTypeRange big_pairs = htr_empty();
	for (int r = TEN; r <= ACE; r++) htr_add(&big_pairs, make_pair((uint8_t)r));
	printf("  %d types, %d max combos\n", htr_count(&big_pairs), htr_combo_count_max(&big_pairs));
	output_htr(&big_pairs);
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
		HtrBoardProfile overview = profile(&bluffs, &game);	
		
		printf("Hands Hero Beats: \n");
		output_htr(&bluffs); printf("\n");
		output_htr_board_profile(&overview); printf("\n\n");
	
		printf("Hands better than Hero: \n");
		output_htr(&value); printf("\n");
		overview = profile(&value, &game);
		output_htr_board_profile(&overview); printf("\n\n");
	}	
} 

void test_alt(void) {	
	uint64_t deck = FULL_DECK;
	Combo playerhands[6];

	// Deal hole cards
	printf("=== Hole Cards ===\n");
	for (int i = 0; i < 6; i++) {
		playerhands[i] = (Combo){ .a = toCard(deal(&deck)), .b = toCard(deal(&deck)) };
		printf("  Player %d:  ", i);
		output_combo(playerhands[i]); printf("\n");
		output_binary(combo_toBitmask(playerhands[i]));
	}
	printf("\n");

	uint64_t board = 0;

	for (int i = 0; i < 3; i++) {
		board |= deal(&deck);
	}

	printf("Board: "); output_board(board);
	printf("\n");
	
	for (int i = 0; i < 6; i++) {
		uint64_t mask = combo_toBitmask(playerhands[i]);
		printf("  Player %d:  ", i);
		output_combo(playerhands[i]); printf("  ");
		printf("0x%08X  ", calculate_hand_strength(board | mask));
		output_hand_rank(classify_hand(board | mask)); printf("\n");
	}

	HandTypeRange htr = htr_full();
	printf("Full HandTypeRange: \n");
	printf("----------------------\n");
	output_htr(&htr); printf("\n");
	
	uint64_t hero = combo_toBitmask(playerhands[0]);

	Range r = range_full();
	range_remove_blocked(&r, merge(board, hero));

	printf("Full Unblocked Combo Range: \n");
	printf("---------------------------------\n");
	output_range(&r); printf("\n");

	printf("HandTypeRange Board Profile: \n");
	printf("-----------------------------\n");
	HtrBoardProfile prof = htr_board_profile(&htr, board);
	output_htr_board_profile(&prof); printf("\n");
	
	printf("Filtering HandTypeRange by rank: \n");
	for (int i = 0; i < 5; i++) {
		printf("--------\n");
		HandTypeRange subhtr = htrfilter_by_rank(&htr, board, i);
		output_htr(&subhtr); printf("\n");
	}		

	printf("Filtering HandTypeRange by draw: \n");
	for (int i = 0; i < 5; i++) {
		printf("--------\n");
		HandTypeRange subhtr = htrfilter_by_draw(&htr, board, i);
		output_htr(&subhtr); printf("\n");
	}		
	
} 
