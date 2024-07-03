#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };

static void
sigchld_handler(int signum)
{
	if (signum == SIGCHLD) {
		pid_t pid;
		int status;

		while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
			char substr1[] = "PID: ";
			char substr2[] = " finished, status: ";

			char buf[ITOA_SAFE_STRLEN(pid_t) + ITOA_SAFE_STRLEN(int) +
			         sizeof(substr1) + sizeof(substr2)];
			enum { base = 10 };

			char *end;
			end = buf;

			strcpy(end, substr1);
			end += sizeof(substr1);
			end = itoa_safe(pid, end, base);
			strcat(end, substr2);
			end += sizeof(substr2);
			end = itoa_safe(status, end, base);
			*end++ = '\n';

			if (write(STDOUT_FILENO, buf, end - buf) < 0)
				return;
		}
	}

	return;
}

static void
set_sigchld_action(void)
{
	stack_t ss = { .ss_size = SIGSTKSZ, .ss_sp = malloc(SIGSTKSZ) };

	struct sigaction sa = { .sa_handler = sigchld_handler,
		                .sa_flags = SA_ONSTACK | SA_RESTART };

	if (sigaltstack(&ss, NULL) < 0) {
		perror("sigaltstack failed");
		_exit(-1);
	}

	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction failed");
		_exit(-1);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	set_sigchld_action();

	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
