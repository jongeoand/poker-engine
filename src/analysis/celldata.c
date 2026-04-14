#include "celldata.h"

CellData cell_analyze(HMapCell cell) {
	CellData d = {0};

	int total = hmap_cell_total(cell);
	if (total == 0) {
		d.empty = true;
		return d;
	}

	/* Dominant ComboState — highest combo count across all suit classes. */
	d.dominant_state = COMBO_AHEAD;
	for (int s = 1; s < COMBO_STATE_COUNT; s++)
		if (hmap_cell_state_total(cell, s) > hmap_cell_state_total(cell, (int)d.dominant_state))
			d.dominant_state = (ComboState)s;

	d.dom_frac  = (double)hmap_cell_state_total(cell, (int)d.dominant_state) / total;
	d.draw_frac = (double)hmap_cell_state_total(cell, COMBO_BEHIND_LIVE) / total;

	/* Dominant SuitClass across all states. */
	d.dominant_suit = SUIT_NONE;
	for (int u = 1; u < SUIT_CLASS_COUNT; u++)
		if (hmap_cell_suit_total(cell, u) > hmap_cell_suit_total(cell, (int)d.dominant_suit))
			d.dominant_suit = (SuitClass)u;

	/* Dominant SuitClass within the dominant state only. */
	d.dominant_suit_in_state = SUIT_NONE;
	for (int u = 1; u < SUIT_CLASS_COUNT; u++)
		if (hmap_cell_get(cell, (int)d.dominant_state, u) >
		    hmap_cell_get(cell, (int)d.dominant_state, (int)d.dominant_suit_in_state))
			d.dominant_suit_in_state = (SuitClass)u;

	/* Flush fraction: draw or made, summed across all states. */
	int flush_n = hmap_cell_suit_total(cell, SUIT_FLUSH_DRAW)
	            + hmap_cell_suit_total(cell, SUIT_FLUSH_MADE);
	d.flush_frac = (double)flush_n / total;

	return d;
}
