#include <stdio.h>
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
#include "cli/panel.c"
#include "cli/views.c"
#include "sim/game.c"

#include "tests/visual/visual_test.c"
#include "tests/visual/visual_experiments.c"

#define SEP "============================================================\n"

int main(void) {
	srand((unsigned int)time(NULL));
	visual_test();         printf(SEP);
	visual_experiments();  printf(SEP);
	return 0;
}
