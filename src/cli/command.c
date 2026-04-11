#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "command.h"

/* Strip trailing whitespace in-place; return pointer past leading whitespace. */
static char* trim(char* s) {
	while (isspace((unsigned char)*s)) s++;
	char* end = s + strlen(s);
	while (end > s && isspace((unsigned char)*(end - 1))) *--end = '\0';
	return s;
}

int cmd_dispatch_key(const CommandTable* t, char key, Session* sesh) {
	for (int i = 0; i < t->count; i++) {
		if (t->cmds[i].key == key)
			return t->cmds[i].fn(sesh, 0, NULL);
	}
	fprintf(stderr, "Unknown command '%c'  (type 'help' for list)\n", key);
	return CMD_ERR;
}

int cmd_dispatch_line(const CommandTable* t, const char* line, Session* sesh) {
	char buf[256];
	strncpy(buf, line, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	char* p = trim(buf);
	if (*p == '\0') return CMD_OK;

	/* Tokenize into argv */
	char* argv[32];
	int   argc = 0;
	char* tok = strtok(p, " \t");
	while (tok && argc < 32) {
		argv[argc++] = tok;
		tok = strtok(NULL, " \t");
	}
	if (argc == 0) return CMD_OK;

	const char* name = argv[0];

	/* Try word-name match first */
	for (int i = 0; i < t->count; i++) {
		if (t->cmds[i].name && strcmp(t->cmds[i].name, name) == 0)
			return t->cmds[i].fn(sesh, argc - 1, argv + 1);
	}

	/* Fall back to single-char key match */
	if (name[0] != '\0' && name[1] == '\0') {
		char key = name[0];
		for (int i = 0; i < t->count; i++) {
			if (t->cmds[i].key == key)
				return t->cmds[i].fn(sesh, argc - 1, argv + 1);
		}
	}

	fprintf(stderr, "Unknown command '%s'  (type 'help' for list)\n", name);
	return CMD_ERR;
}

void cmd_print_help(const CommandTable* t, FILE* out) {
	for (int i = 0; i < t->count; i++) {
		const Command* c = &t->cmds[i];
		char key_str[4] = "   ";
		if (c->key) { key_str[0] = '['; key_str[1] = c->key; key_str[2] = ']'; }
		fprintf(out, "  %-3s  %-20s  %s\n", key_str, c->usage ? c->usage : "", c->help);
	}
}
