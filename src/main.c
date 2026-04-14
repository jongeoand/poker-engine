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
#include "analysis/celldata.c"
#include "map/handmap.c"
#include "cli/symbols.c"
#include "cli/panel.c"
#include "cli/views.c"
#include "sim/game.c"
#include "cli/command.c"
#include "cli/session.c"

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
// Top-level launchboard
// ============================================================

static void run_session_cmd(void) {
	Session sesh = session_default();
	start_session(&sesh);
}

static void print_launchboard_prompt(void) {
	printf("=== Launchboard ===\n");
	printf("  s -  session\n");
	printf("  q -  quit\n\n > ");
}

static const Cmd launchboard_cmds[] = {
	{ 's', "session", run_session_cmd },
};

#define LAUNCHBOARD_CMD_COUNT 1

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
		printf("Unknown command '%s'\n", argv[1]);
        return 0;
	//	for (int i = 0; i < TEST_CMD_COUNT; i++) {
	//		if (key == test_cmds[i].key) {
	//			test_cmds[i].fn();
	//			return 0;
	//		}
	//	}
	//	printf("Available:");
	//	for (int i = 0; i < TEST_CMD_COUNT; i++)
	//		printf("  %c (%s)", test_cmds[i].key, test_cmds[i].label);
	//	printf("\n");
	//	return 1;
	}

	print_file("src/res/title.txt");
	print_file("src/res/splash.txt"); printf("\n");
	launchboard();
	return 0;
}
