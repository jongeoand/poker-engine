#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "core/card.c"
#include "core/combo.c"
#include "core/handtype.c"
#include "engine/engine.c"
#include "engine/eval.c"
#include "engine/draws.c"
#include "range/range.c"
#include "range/htrange.c"
#include "cli/render.c"
#include "utility.c"
#include "range/iterate.c"
#include "analysis/combostate.c"
#include "map/handmap.c"
#include "cli/symbols.c"
#include "cli/views.c"
#include "cli/output.c"
#include "sim/game.c"

#include "tests/test_core.c"
#include "tests/test_range.c"
#include "tests/test_engine.c"
#include "tests/test_combostate.c"
#include "tests/test_handmap.c"
#include "tests/test_render.c"

#define SEP "============================================================\n"

int main(void) {
	srand((unsigned int)time(NULL));

	print_struct_sizes(); printf(SEP);
	test_cards();         printf(SEP);
	test_combos();        printf(SEP);
	test_handtypes();     printf(SEP);
	test_range();         printf(SEP);
	test_handtype_range(); printf(SEP);
	test_engine();         printf(SEP);
	test_combostate();     printf(SEP);
	test_handmap(); printf(SEP);
	test_render();

	return 0;
}
