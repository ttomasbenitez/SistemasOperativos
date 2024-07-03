#ifndef BUILTIN_H
#define BUILTIN_H

#define CD_COMMAND "cd"
#define PWD_COMMAND "pwd"
#define EXIT_COMMAND "exit"

#define PWD_COMMAND_CAP "PWD"
#define EXIT_COMMAND_CAP "EXIT"

#include "defs.h"
#include <ctype.h>


extern char prompt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

int history(char *cmd);

#endif  // BUILTIN_H
