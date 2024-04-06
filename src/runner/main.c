#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

void execute_external_program(char **arguments)
{
	pid_t pid = fork(); // Se crea el proceso hijo como copia exacta del padre

	if (pid < 0) // Si fork devuelve un numero negativo, hubo un error creando al hijo
	{
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) // Si devuelve 0 es porque estamos en el proceso hijo
	{
		execvp(arguments[0], arguments); // Se ejecuta el programa externo
		perror("Exec failed");			 // Si execvp retorna, hubo un error
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char const *argv[])
{
	/*Lectura del input*/
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	// Obtención de parámetros de la línea de comandos
	char *output_file = (char *)argv[2];
	int amount = atoi(argv[3]);
	int max = (argc > 4) ? atoi(argv[4]) : -1; // Valor por defecto: tiempo ilimitado

	// Variables para manejar el tiempo límite
	time_t start_time;
	time_t current_time;

	/*Mostramos el archivo de input en consola*/
	// Se itera sobre las lineas del archivo de entrada
	printf("Cantidad de lineas: %d\n", input_file->len);
	for (int i = 0; i < input_file->len; ++i)
	{
		char **line = input_file->lines[i];
		int argc = atoi(line[0]);
		char **arguments = &line[1];

		printf("%d ", atoi(input_file->lines[i][0]));
		printf("%s ", input_file->lines[i][1]);
		for (int j = 2; j < argc + 2; ++j)
		{
			printf("%s ", input_file->lines[i][j]);
		}

		printf("\n");

		// Ejecutamos el programa externo si no es un comando "wait all"
		if (strcmp(arguments[0], "wait") != 0 || strcmp(arguments[1], "all") != 0)
		{
			execute_external_program(arguments);
			amount--; // Decrementamos el contador de procesos permitidos
		}
		else
		{
			// Comando "wait all"
			int timeout = atoi(arguments[2]);

			// Esperamos hasta que se cumpla el tiempo límite o se terminen todos los procesos
			start_time = time(NULL);
			while (amount < 0 && (max < 0 || difftime(time(NULL), start_time) < max))
			{
				if (waitpid(-1, NULL, WNOHANG) > 0)
				{
					amount++; // Incrementamos el contador de procesos permitidos
				}
			}

			// Si todavía hay procesos en ejecución después del tiempo límite, los matamos
			if (amount < 0 && max >= 0)
			{
				kill(-1, SIGKILL);
			}
		}

		// Si se ha alcanzado el límite de procesos permitidos, esperamos a que termine uno
		while (amount <= 0)
		{
			wait(NULL);
			amount++;
		}
	}

	// Esperamos a que terminen todos los procesos restantes
	while (amount < 0)
	{
		wait(NULL);
		amount++;
	}

	input_file_destroy(input_file);

	return 0;
}