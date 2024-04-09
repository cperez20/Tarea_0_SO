#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <stdbool.h>
#include "../file_manager/manager.h"
#include "../son_process/process.h"
#include "../son_process/listprocess.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

ProcessList *list; // lista que contendra a los procesos hijos
int list_len = 0;
int amount;
int count = 0;

void execute_external_program(char **arguments, bool first_process)
{
	pid_t pid = fork(); // Se crea el proceso hijo como copia exacta del padre

	if (pid < 0) // Si fork devuelve un numero negativo, hubo un error creando al hijo
	{
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) // Si devuelve 0 es porque estamos en el proceso hijo
	{
		if (execvp(arguments[0], arguments) == -1)
		{
			// Si execvp retorna, entonces hubo un error.
			perror("Exec failed");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		double son_created = clock();																 // Obtenemos hora de creacion
		SonProcess process;																			 // Pedimos memoria para el struct
		process = (SonProcess){.pid = pid, .executable = arguments[0], .time_created = son_created}; // Inicializamos sus valores
		// Analizamos si debemos inicializar la lista ligada o agregarle un valor
		if (first_process)
		{
			list = processlist_init(process);
		}
		else
		{
			processlist_append(list, process);
		}
		list_len = list_len + 1; // Aumentamos el largo de la lista
	}
}

void ended_handler(pid_t son_pid, int status)
{

	for (int i = 0; i < list_len; i++)
	{
		SonProcess *son = processlist_at_index(list, i);
		if (son->pid == son_pid)
		{							   // Encontramos al proceso que acaba de terminar
			son->time_ended = clock(); // Asignamos valor a ended
			// Asignamos valor a status
			if (WIFEXITED(status))
			{
				son->status = WEXITSTATUS(status);
			}
			else if (WIFSIGNALED(status))
			{
				son->status = WTERMSIG(status);
			}
			break;
		}
	}
}

void child_termination_handler(int sig)
{
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		amount++;
		count++;

		for (int i = 0; i < list_len; i++)
		{
			SonProcess *son = processlist_at_index(list, i);
			if (son->pid == pid)
			{
				son->time_ended = clock();
				if (WIFEXITED(status))
				{
					son->status = WEXITSTATUS(status);
				}
				else if (WIFSIGNALED(status))
				{
					son->status = WTERMSIG(status);
				}
				break;
			}
		}
	}
}

void sigterm_handler(int sig)
{
	int pid;
	int status;
	pid = waitpid(-1, &status, 0);
	if (pid != 0)
	{
		printf("10 segs alcanzado. Enviando SIGTERM a todos los procesos restantes.\n");
		kill(-1, SIGTERM);
	}
}

// Definimos variable para decidir si la alarma de wait esta activa o no
bool wait_alarm = false;
void timeout_handler(int sig)
{
	if (wait_alarm)
	{
		printf("Timeout alcanzado. Enviando SIGKILL a todos los procesos restantes.\n");
		kill(-1, SIGKILL); // Termina todos los procesos hijo restantes
	}
	else
	{
		printf("Max alcanzado. Enviando SIGINT a todos los procesos restantes.\n");
		kill(-1, SIGINT);
		signal(SIGALRM, sigterm_handler); // Definimos nueva alarma
		alarm(10);
	}
}

int main(int argc, char const *argv[])
{
	/*Lectura del input*/
	// Formato de lectura /. runner {input} {output} {amount} [{max}]
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	// Obtención de parámetros de la línea de comandos
	char *output_file = (char *)argv[2];
	FILE *csv_file = fopen(output_file, "w");
	amount = atoi(argv[3]);
	int original_amount = amount;			   // Guardamos variable de amount que no cambiara
	int max = (argc > 4) ? atoi(argv[4]) : -1; // Valor por defecto: tiempo ilimitado
	bool first_process = true;				   // Registra si el primer proceso ya empezo (nos sirve para inicializar la lista ligada)

	signal(SIGCHLD, child_termination_handler); // Funcion que se activara cada vez que termine un proceso hijo

	// Si max tiene un valor distinto de 1 se le asigna ese valor a la alarma
	if (max != -1)
	{
		signal(SIGALRM, timeout_handler);
		alarm(max); // Configuramos un temporizador
	}

	/*Mostramos el archivo de input en consola*/
	// Se itera sobre las lineas del archivo de entrada
	printf("Cantidad de lineas: %d\n", input_file->len);
	for (int i = 0; i < input_file->len; ++i)
	{
		char **line = input_file->lines[i];
		int argc = atoi(line[0]);	 // Numero de argumentos
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
				amount = amount + 1;
			}
			execute_external_program(arguments, first_process);
			first_process = false;

			amount = amount - 1; // Decrementamos el contador de procesos permitidos
		}
		else
		{
			int timeout = atoi(arguments[1]); // Obtiene el timeout del comando wait_all
			pid_t pid;
			int status;
			time_t set_time;

			// Obtén el tiempo restante de la alarma y cancela la alarma actual
			unsigned int seconds_left = alarm(0);
			// Si timeout se va a disparar antes que max se setea esa alarma
			if (seconds_left > timeout)
			{
				set_time = time(NULL);
				alarm(timeout);
				wait_alarm = true;
			}
			else
			{
				alarm(seconds_left);
			}

			while (true)
			{

				// Espera a cualquier proceso hijo hasta que uno termine o hasta que se alcance el timeout
				pid = waitpid(-1, &status, 0); // 0 para bloquear hasta que un proceso hijo termine

				if (pid > 0)
				{
					// Un proceso hijo ha terminado
					printf("Proceso %d terminado.\n", pid);
				}
				else if (pid == -1)
				{
					// Error en waitpid o no hay más procesos hijo por esperar
					break;
				}
			}

			// Volvemos a setear la alarma global
			if (wait_alarm)
			{
				time_t new_seconds = time(NULL) - set_time;
				alarm(new_seconds);
				wait_alarm = false;
			}

			// Después de alcanzar el timeout, asegúrate de que todos los procesos hijo sean recogidos (evita procesos zombis)
			while (waitpid(-1, NULL, 0) > 0)
				;
		}
	}

	// Esperamos a que terminen todos los procesos restantes
	while (amount < original_amount)
	{
		int status;
		pid_t child_pid = waitpid(-1, &status, WNOHANG);
		if (child_pid > 0)
		{
			printf("Entró child pid %i\n", child_pid);
			ended_handler(child_pid, status);
			amount = amount + 1;
		}
	}

	// Escribimos en el outputfile
	for (int i = 0; i < list_len; i++)
	{
		SonProcess *son = processlist_at_index(list, i);
		double elapsed_time = (double)(son->time_ended - son->time_created) / CLOCKS_PER_SEC;
		fprintf(csv_file, "%s,%f,%d\n", son->executable, elapsed_time, son->status);
		printf("Elapsed time process %d: %f\n", i + 1, elapsed_time);
	}

	printf("Entro: %d a la señal\n", count);
	fclose(csv_file);
	input_file_destroy(input_file);
	processlist_destroy(list);

	return 0;
}