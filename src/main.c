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
#include "cli/command.c"
#include "cli/session.c"
#include "tests/test_core.c"
#include "tests/test_range.c"
#include "tests/test_engine.c"
#include "tests/test_combostate.c"
#include "tests/test_handmap.c"

void print_file(const char* filename) {
	FILE* f = fopen(filename, "r");
	if (f == NULL) { printf("Error opening file: %s\n", filename); exit(1); }
	int c;
	while ((c = fgetc(f)) != EOF) putchar(c);
	fclose(f);
}

bool read_command(char* out) {
	char line[64];
	if (!fgets(line, sizeof(line), stdin)) return false;
	*out = '\0';
	for (int i = 0; line[i]; i++) {
		if (line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
			*out = line[i];
			break;
		}
	}
	return true;
}

typedef struct { char key; const char* label; void (*fn)(void); } Cmd;

void run_menu(void (*print_prompt)(void), const Cmd* cmds, int n) {
	print_prompt();
	char c;
	while (read_command(&c)) {
		if (c == '\0') { print_prompt(); continue; }
		if (c == 'q')  break;
		bool found = false;
		for (int i = 0; i < n; i++) {
			if (c == cmds[i].key) { cmds[i].fn(); found = true; break; }
		}
		if (!found) printf("Unknown command '%c'\n\n", c);
		print_prompt();
	}
}

// ============================================================
// Tests submenu
// ============================================================

static void print_tests_prompt(void) {
	printf("=== Tests ===\n");
	printf("  1 -  struct sizes\n");
	printf("  2 -  cards\n");
	printf("  3 -  combos\n");
	printf("  4 -  hand types\n");
	printf("  5 -  range\n");
	printf("  6 -  hand type range\n");
	printf("  7 -  combostate & streams\n");
	printf("  8 -  handmap\n");
	printf("  n -  test new \n");
	printf("  q -  back\n\n > ");
}

static const Cmd test_cmds[] = {
	{ '1', "struct sizes",    print_struct_sizes    },
	{ '2', "cards",           test_cards            },
	{ '3', "combos",          test_combos           },
	{ '4', "hand types",      test_handtypes        },
	{ '5', "range",           test_range            },
	{ '6', "hand type range", test_handtype_range   },
	{ '7', "combostate",      test_combostate       },
	{ '8', "handmap",         test_handmap          },
	{ '9', "engine",             test_engine           },
};
#define TEST_CMD_COUNT 9

static void run_tests(void) {
	run_menu(print_tests_prompt, test_cmds, TEST_CMD_COUNT);
}

// ============================================================
// Top-level launchboard
// ============================================================

static void run_session_cmd(void) {
	Session sesh = session_default();
	start_session(&sesh);
}

static void print_launchboard_prompt(void) {
	printf("=== Launchboard ===\n");
	printf("  s -  session\n");
	printf("  t -  tests\n");
	printf("  q -  quit\n\n > ");
}

static const Cmd launchboard_cmds[] = {
	{ 's', "session", run_session_cmd },
	{ 't', "tests",   run_tests       },
};
#define LAUNCHBOARD_CMD_COUNT 2

void launchboard(void) {
	run_menu(print_launchboard_prompt, launchboard_cmds, LAUNCHBOARD_CMD_COUNT);
}

// ============================================================
// Entry point
// ============================================================

int main(int argc, char* argv[]) {
	srand((unsigned int)time(NULL));

	// Direct dispatch: ./poker 1..5 runs that test and exits.
	if (argc > 1) {
		char key = argv[1][0];
		for (int i = 0; i < TEST_CMD_COUNT; i++) {
			if (key == test_cmds[i].key) {
				test_cmds[i].fn();
				return 0;
			}
		}
		printf("Unknown command '%s'\n", argv[1]);
		printf("Available:");
		for (int i = 0; i < TEST_CMD_COUNT; i++)
			printf("  %c (%s)", test_cmds[i].key, test_cmds[i].label);
		printf("\n");
		return 1;
	}

	print_file("src/res/title.txt");
	print_file("src/res/splash.txt"); printf("\n");
	launchboard();
	return 0;
}
