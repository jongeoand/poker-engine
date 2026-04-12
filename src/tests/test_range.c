void test_range(void) {
	printf("=== Range ===\n\n");
	Renderer ren = render_default();

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
		render_card(&ren, card);
		printf(": %4d combos\n", range_count(&r));
	}

	printf("\nFull range (%d combos):\n\n", range_count(&r));
	bool first = true;
	for (int i = 0; i < COMBO_RANGE_WORDS; i++) {
		uint32_t word = r.bits[i];
		while (word) {
			int bit = __builtin_ctz(word); word &= word - 1;
			int idx = i * 32 + bit;
			if (idx >= COMBO_COUNT) break;
			if (!first) printf(" ");
			render_combo(&ren, combo_from_index(idx));
			first = false;
		}
	}
	printf("\n");
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
