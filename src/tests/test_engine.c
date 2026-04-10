void test_engine(void) {
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
