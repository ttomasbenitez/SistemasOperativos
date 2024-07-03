#include "builtin.h"
#include <errno.h>
#include "defs.h"
#include "utils.h"

static void
execute_chdir(char *path)
{
	char buf[BUFLEN] = { 0 };

	if (chdir(path) < 0) {
		fprintf(stderr, "chdir failed: %s\n", strerror(errno));
		perror(buf);
	}
}

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, EXIT_COMMAND) == 0 || strcmp(cmd, EXIT_COMMAND_CAP) == 0) {
		exit(3);
	}

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd == strstr(cmd, "cd")) {
		char *dir = split_line(cmd, ' ');

		if (*dir == END_STRING) {
			char *home = getenv("HOME");
			execute_chdir(home);
		} else {
			execute_chdir(dir);
		}

		snprintf(prompt, sizeof prompt, "(%s)", getcwd(NULL, 0));

		return 1;
	} else {
		return 0;
	}
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, PWD_COMMAND) == 0 || strcmp(cmd, PWD_COMMAND_CAP) == 0) {
		char cwd[1024];

		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("%s\n", cwd);
		} else {
			perror("error");
			return 0;
		}

		return 1;
	} else {
		return 0;
	}
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
