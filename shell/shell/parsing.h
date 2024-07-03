#ifndef PARSING_H
#define PARSING_H

#include "defs.h"
#include "types.h"
#include "createcmd.h"
#include "utils.h"

#define ENV_VAR_IDENTIFIER '$'
#define MAGICAL_VAR '?'

extern int status;

struct cmd *parse_line(char *b);

#endif  // PARSING_H
