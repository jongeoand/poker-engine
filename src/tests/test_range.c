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
