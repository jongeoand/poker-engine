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
#include "range/htrange.c"
#include "cli/render.c"
#include "range/iterate.c"
#include "analysis/combostate.c"
#include "map/handmap.c"
#include "cli/symbols.c"
#include "cli/views.c"
#include "cli/panel.c"
#include "sim/game.c"
#include "cli/session.h"
#include "cli/command.h"

#include "tests/test_core.c"
#include "tests/test_range.c"
#include "tests/test_combostate.c"
#include "tests/test_handmap.c"
#include "tests/test_render.c"
#include "tests/test_panel.c"

#define SEP "============================================================\n"

int main(void) {
	srand((unsigned int)time(NULL));

	print_struct_sizes(); printf(SEP);
	test_cards();         printf(SEP);
	test_combos();        printf(SEP);
	test_handtypes();     printf(SEP);
	test_range();         printf(SEP);
	test_handtype_range(); printf(SEP);
	test_combostate();     printf(SEP);
	test_handmap(); printf(SEP);
	test_render();  printf(SEP);
	test_panel();

	return 0;
}
