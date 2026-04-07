#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "tests.h"

typedef struct { char key; const char* label; void (*fn)(void); } Cmd;

bool read_command(char* out);
void run_menu(void (*print_prompt)(void), const Cmd* cmds, int n);
void print_file(const char* filename);
void launchboard(void);

#endif
