#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <stdbool.h>
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

void timeout_handler(int sig) {
    printf("Timeout alcanzado. Enviando SIGKILL a todos los procesos restantes.\n");
    kill(-1, SIGKILL); // Termina todos los procesos hijo restantes
}

int main(int argc, char const *argv[])
{
	/*Lectura del input*/
	// Formato de lectura /. runner {input} {output} {amount} [{max}]
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
		int argc = atoi(line[0]); // Numero de argumentos
		char **arguments = &line[1]; // Obtiene nombre del ejecutable y argumentos

		printf("%d ", atoi(input_file->lines[i][0]));
		printf("%s ", input_file->lines[i][1]);
		for (int j = 2; j < argc + 2; ++j)
		{
			printf("%s ", input_file->lines[i][j]);
		}

		printf("\n");

		// Ejecutamos el programa externo si no es un comando "wait_all"
		if (strcmp(arguments[0], "wait_all") != 0)
		{
			// Si se ha alcanzado el límite de procesos permitidos, esperamos a que termine uno
			while (amount == 0)
			{
				wait(NULL);
				amount++;
			}
			execute_external_program(arguments);
			amount--; // Decrementamos el contador de procesos permitidos
		}
		else
		{
			int timeout = atoi(arguments[1]); // Obtiene el timeout del comando wait_all
			time_t start_time = time(NULL);
			pid_t pid;
			int status;

			// Usamos señales para manejar el timeout
			signal(SIGALRM, timeout_handler);
			alarm(timeout);

    		while (true) {

				// Espera a cualquier proceso hijo hasta que uno termine o hasta que se alcance el timeout
				pid = waitpid(-1, &status, 0); // 0 para bloquear hasta que un proceso hijo termine

				if (pid > 0) {
					// Un proceso hijo ha terminado
					printf("Proceso %d terminado.\n", pid);
					amount ++; // Aumentamos el contador de procesos permitidos
				} else if (pid == -1) {
					// Error en waitpid o no hay más procesos hijo por esperar
					break;
				}
    		}

    		// Después de alcanzar el timeout, asegúrate de que todos los procesos hijo sean recogidos (evita procesos zombis)
    		while (waitpid(-1, NULL, 0) > 0);
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