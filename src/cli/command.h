#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdio.h>

/* Forward declaration — full definition in session.h */
typedef struct Session Session;

typedef int (*CmdFn)(Session* sesh, int argc, char** argv);

#define CMD_OK   0
#define CMD_QUIT 1
#define CMD_ERR  2

typedef struct {
	const char* name;   /* word name for line dispatch (NULL = key-only) */
	char        key;    /* single-char shorthand ('\0' = name-only) */
	const char* usage;  /* brief usage hint, e.g. "range [AKs QQ ...]" */
	const char* help;   /* one-line description */
	CmdFn       fn;
} Command;

typedef struct {
	const Command* cmds;
	int            count;
} CommandTable;

/* Dispatch a single-character keystroke against the table. */
int  cmd_dispatch_key(const CommandTable* t, char key, Session* sesh);

/* Tokenize a line and dispatch by command name or single-char key.
   The line is mutated (strtok). Returns CMD_OK/CMD_QUIT/CMD_ERR. */
int  cmd_dispatch_line(const CommandTable* t, const char* line, Session* sesh);

/* Print a formatted help listing to out. */
void cmd_print_help(const CommandTable* t, FILE* out);

#endif
