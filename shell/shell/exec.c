#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char env_key[ARGSIZE];
		char env_val[ARGSIZE];
		get_environ_key(eargv[i], env_key);
		get_environ_value(eargv[i], env_val, block_contains(eargv[i], '='));
		setenv(env_key, env_val, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	mode_t mode = 0;
	int fd;

	if (flags & O_CREAT) {
		mode = S_IWUSR | S_IRUSR;
	}

	if (file[0] == '&') {
		fd = STDOUT_FILENO;
	} else {
		fd = open(file, flags, mode);

		if (fd < 0) {
			perror("open failed");
			_exit(-1);
		}
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		//
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		execvp(e->argv[0], e->argv);

		perror("exec failed");
		_exit(-1);
	}

	case BACK: {
		// runs a command in background
		//
		b = (struct backcmd *) cmd;

		pid_t pid = fork();

		if (pid < 0) {
			perror("fork failed");
			_exit(-1);
		}

		if (pid == 0) {
			exec_cmd(b->c);
		}

		int status;
		waitpid(b->c->pid, &status, 0);

		exit(status);
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//

		r = (struct execcmd *) cmd;

		set_environ_vars(r->eargv, r->eargc);

		if (strlen(r->out_file) > 0) {
			int fd_out = open_redir_fd(r->out_file,
			                           O_CREAT | O_WRONLY |
			                                   O_TRUNC | O_CLOEXEC);

			if (dup2(fd_out, STDOUT_FILENO) < 0) {
				perror("dup failed");
				_exit(-1);
			}
		}

		if (strlen(r->in_file) > 0) {
			int fd_in =
			        open_redir_fd(r->in_file, O_RDONLY | O_CLOEXEC);

			if (dup2(fd_in, STDIN_FILENO) < 0) {
				perror("dup failed");
				_exit(-1);
			}
		}

		if (strlen(r->err_file) > 0) {
			int fd_err = open_redir_fd(r->err_file,
			                           O_CREAT | O_WRONLY |
			                                   O_TRUNC | O_CLOEXEC);

			if (dup2(fd_err, STDERR_FILENO) < 0) {
				perror("dup failed");
				_exit(-1);
			}
		}

		execvp(r->argv[0], r->argv);

		perror("exec failed");
		_exit(-1);
	}

	case PIPE: {
		// pipes two commands
		//

		p = (struct pipecmd *) cmd;

		int cmd_pipe[2];

		if (pipe(cmd_pipe) < 0) {
			perror("pipe failed");
			_exit(-1);
		}

		pid_t lchild_pid = fork();

		if (lchild_pid < 0) {
			perror("fork failed");
			_exit(-1);
		}

		if (lchild_pid == 0) {
			if (setpgid(0, 0) < 0) {
				perror("setpgid failed");
				_exit(-1);
			}

			close(cmd_pipe[READ]);

			if (dup2(cmd_pipe[WRITE], STDOUT_FILENO) < 0) {
				perror("dup failed");
				_exit(-1);
			}

			close(cmd_pipe[WRITE]);

			exec_cmd(p->leftcmd);
		}

		pid_t rchild_pid = fork();

		if (rchild_pid < 0) {
			perror("fork failed");
			_exit(-1);
		}

		if (rchild_pid == 0) {
			if (setpgid(0, 0) < 0) {
				perror("setpgid failed");
				_exit(-1);
			}

			close(cmd_pipe[WRITE]);

			if (dup2(cmd_pipe[READ], STDIN_FILENO) < 0) {
				perror("dup failed");
				_exit(-1);
			}

			close(cmd_pipe[READ]);

			exec_cmd(p->rightcmd);
		}

		close(cmd_pipe[READ]);
		close(cmd_pipe[WRITE]);

		int lstatus, rstatus;

		if (waitpid(lchild_pid, &lstatus, 0) < 0) {
			perror("wait failed");
			_exit(-1);
		}

		if (waitpid(rchild_pid, &rstatus, 0) < 0) {
			perror("wait failed");
			_exit(-1);
		}

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		exit(rstatus);
	}
	}
}
