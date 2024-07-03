#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#ifndef NARGS
#define NARGS 4
#endif

#define EOS '\0'
#define POS_COMANDO 0


void
ejecutar_comando(char *cmd_y_args[])
{
	pid_t pid = fork();

	if (pid < 0) {
		perror("Error en fork\n");
		exit(EXIT_FAILURE);

	} else if (pid == 0) {
		if (execvp(cmd_y_args[POS_COMANDO], cmd_y_args) < 0) {
			perror("Error en execvp\n");
			exit(EXIT_FAILURE);
		}

	} else {
		wait(NULL);
	}
}


void
liberar_memoria_args(char *cmd_y_args[], int cantidad_args)
{
	for (int i = 1; i <= cantidad_args; i++) {
		free(cmd_y_args[i]);
	}
}


void
ejecutar_y_liberar_args(char *cmd_y_args[], int cantidad_args)
{
	cmd_y_args[cantidad_args + 1] = EOS;
	ejecutar_comando(cmd_y_args);
	liberar_memoria_args(cmd_y_args, cantidad_args);
}


void
procesar_args_entrada(char *cmd_y_args[])
{
	char *linea = NULL;
	size_t tamanio = 0;

	int cantidad_args = 0;
	int cant_chars = 0;

	while ((cant_chars = getline(&linea, &tamanio, stdin)) > 0) {
		if (linea[cant_chars - 1] == '\n') {
			linea[cant_chars - 1] = EOS;
		}

		cmd_y_args[cantidad_args + 1] = strdup(linea);
		cantidad_args++;

		if (cantidad_args == NARGS) {
			ejecutar_y_liberar_args(cmd_y_args, cantidad_args);
			cantidad_args = 0;
		}
	}

	if (cantidad_args > 0) {
		ejecutar_y_liberar_args(cmd_y_args, cantidad_args);
	}

	free(linea);
}


int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Faltan argumentos\n");
		return 1;
	}

	char *cmd_y_args[NARGS + 2];
	cmd_y_args[POS_COMANDO] = argv[1];

	procesar_args_entrada(cmd_y_args);


	return (0);
}
