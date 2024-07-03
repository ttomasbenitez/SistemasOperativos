#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define FD_READ 0
#define FD_WRITE 1


void
check_error_write(ssize_t retorno_write)
{
	if (retorno_write < 0) {
		perror("Error en write\n");
		exit(EXIT_FAILURE);
	}
}


void
filtrar_numeros_primos(int fd_lectura)
{
	int primo;
	int fd[2];


	if (read(fd_lectura, &primo, sizeof(primo)) == 0) {
		// Cierro siempre las opciones de fd que no voy a usar para gestionar recursos.
		close(fd_lectura);
		exit(EXIT_FAILURE);
	}
	printf("primo %d\n", primo);
	fflush(stdout);


	if (pipe(fd) < 0) {
		close(fd_lectura);
		perror("Error en pipe\n");
		exit(EXIT_FAILURE);
	}


	pid_t pid = fork();

	if (pid < 0) {
		close(fd[FD_READ]);
		close(fd[FD_WRITE]);
		close(fd_lectura);
		perror("Error en fork\n");
		exit(EXIT_FAILURE);

	} else if (pid == 0) {
		close(fd[FD_WRITE]);
		close(fd_lectura);
		filtrar_numeros_primos(fd[FD_READ]);
		close(fd[FD_READ]);

	} else {
		close(fd[FD_READ]);

		int i;
		while (read(fd_lectura, &i, sizeof(i)) > 0) {
			if (i % primo != 0) {
				check_error_write(
				        write(fd[FD_WRITE], &i, sizeof(i)));
			}
		}

		close(fd_lectura);
		close(fd[FD_WRITE]);
		wait(NULL);  // Espero que terminen los procesos hijos para que no queden procesos zombies.
	}
}


void
obtener_numeros_primos(int numeros)
{
	int fd[2];


	if (pipe(fd) < 0) {
		perror("Error en pipe\n");
		exit(EXIT_FAILURE);
	}


	pid_t pid = fork();

	if (pid < 0) {
		close(fd[FD_READ]);
		close(fd[FD_WRITE]);
		perror("Error en fork\n");
		exit(EXIT_FAILURE);

	} else if (pid == 0) {
		close(fd[FD_WRITE]);
		filtrar_numeros_primos(fd[FD_READ]);

	} else {
		close(fd[FD_READ]);

		for (int i = 2; i <= numeros; i++) {
			check_error_write(write(fd[FD_WRITE], &i, sizeof(i)));
		}

		close(fd[FD_WRITE]);
		wait(NULL);
	}
}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Es necesario un solo argumento\n");
		return 1;
	}

	int numeros = atoi(argv[1]);
	obtener_numeros_primos(numeros);


	return 0;
}
