void test_engine(void) {
	Game game = make_game(6);
	deal_players(&game);
	Renderer ren = render_default();

	HandTypeRange range = htr_full();
	views_htr_grid(&ren, &range); printf("\n");

	Combo hero = game.playerhands[0];
	printf("Hero: "); render_combo(&ren, hero);
	printf("----------------------------\n");

	for (int s = 0; s < 3; s++) {
		deal_street(&game);
		render_board(&ren, game.board); printf("\n");
		printf("Hero: "); render_combo(&ren, hero); printf("\n\n\n");

		HandTypeRange value = behind(&game, 0);
		HandTypeRange bluffs = aheadof(&game, 0);

		printf("Hands Hero Beats: \n");
		views_htr_grid(&ren, &bluffs); printf("\n\n");

		printf("OESD / Gutshots: \n");
		HandTypeRange straight = straight_draws(&game, 0);
		views_htr_grid(&ren, &straight); printf("\n\n");

		if (s == 0) {
			printf("Backdoor straight draws: \n");
			HandTypeRange backdoors = backdoor_straights(&game, 0);
			views_htr_grid(&ren, &backdoors); printf("\n\n");
		}

		printf("Hands better than Hero: \n");
		views_htr_grid(&ren, &value); printf("\n");
	}
}
