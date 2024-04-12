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
#include <math.h>

ProcessList *list;
int list_len = 0;
int amount;
int count = 0;
int term_process = 0;
bool wait_alarm = false;

void execute_external_program(char **arguments, bool first_process)
{
	pid_t pid = fork();

	if (pid < 0)
	{
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (execvp(arguments[0], arguments) == -1)
		{

			perror("Exec failed");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		double son_created = clock();																 // Obtenemos hora de creacion
		SonProcess process;																			 // Pedimos memoria para el struct
		process = (SonProcess){.pid = pid, .executable = arguments[0], .time_created = son_created}; // Inicializamos sus valores

		if (first_process)
		{
			list = processlist_init(process);
		}
		else
		{
			processlist_append(list, process);
		}
		list_len = list_len + 1;
	}
}

void ended_handler(pid_t son_pid, int status)
{
	amount = amount + 1;
	for (int i = 0; i < list_len; i++)
	{
		SonProcess *son = processlist_at_index(list, i);
		if (son->pid == son_pid)
		{
			son->time_ended = clock();
			int elapsed_time = (int)round(son->time_ended - son->time_created) / CLOCKS_PER_SEC;
			printf("SE TERMINO EL PROCESO %d A LOS %d\n", son_pid, elapsed_time);

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

	pid = waitpid(-1, &status, WNOHANG);
	if (pid > 0)
	{
		for (int i = 0; i < list_len; i++)
		{
			SonProcess *son = processlist_at_index(list, i);
			if (son->pid == pid)
			{
				son->time_ended = clock();
				amount = amount + 1;
				int elapsed_time = (int)round(son->time_ended - son->time_created) / CLOCKS_PER_SEC;
				printf("SE TERMINO EL PROCESO %d A LOS %d\n", pid, elapsed_time);
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
	term_process = term_process + 1;
	printf("SE SUMO UN PROCESO TERMINADO\n");
}

void alarm_handler(int sig)
{
	printf("10 segs alcanzados. Enviando SIGTERM a todos los procesos restantes.\n");
	for (int i = 0; i < list_len; i++)
	{
		SonProcess *son = processlist_at_index(list, i);
		kill(son->pid, SIGTERM);
		signal(SIGTERM, child_termination_handler);
	}
}

void timeout_handler(int sig)
{
	if (wait_alarm)
	{
		printf("Timeout alcanzado. Enviando SIGKILL a todos los procesos restantes.\n");
		kill(-1, SIGKILL);
	}
	else
	{
		printf("Max alcanzado. Enviando SIGINT a todos los procesos restantes.\n");
		kill(-1, SIGINT);
		alarm(10);
		signal(SIGALRM, alarm_handler);
	}
}

void ctrlz_handler(int sig)
{
	int status;
	pid_t pid;

	pid = waitpid(-1, &status, WNOHANG);
	if (pid > 0)
	{
		for (int i = 0; i < list_len; i++)
		{
			SonProcess *son = processlist_at_index(list, i);
			if (son->pid == pid)
			{
				son->time_ended = clock();
				amount = amount + 1;
				int elapsed_time = (int)round(son->time_ended - son->time_created) / CLOCKS_PER_SEC;
				printf("SE TERMINO EL PROCESO %d A LOS %d\n", pid, elapsed_time);
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
	term_process = term_process + 1;
	printf("SE SUMO UN PROCESO TERMINADO\n");
	if (term_process < list_len)
	{
		alarm(10);
		wait_alarm = true;
		signal(SIGALRM, timeout_handler);
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

	signal(SIGCHLD, child_termination_handler);
	signal(SIGTSTP, child_termination_handler);

	if (max != -1)
	{
		signal(SIGALRM, timeout_handler);
		alarm(max);
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

		if (strcmp(arguments[0], "wait_all") != 0)
		{

			while (amount == 0)
			{
				int status;
				pid_t child_pid = waitpid(-1, &status, WNOHANG);
				if (child_pid > 0)
				{
					printf("Entró child pid %i\n", child_pid);
					ended_handler(child_pid, status);
				}
			}
			execute_external_program(arguments, first_process);
			first_process = false;

			amount = amount - 1;
		}
		else
		{
			int timeout = atoi(arguments[1]);
			pid_t pid;
			int status;
			time_t set_time;

			unsigned int seconds_left = alarm(0);

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

				pid = waitpid(-1, &status, 0);

				if (pid > 0)
				{

					printf("Proceso %d terminado.\n", pid);
				}
				else if (pid == -1)
				{

					break;
				}
			}

			if (wait_alarm)
			{
				time_t new_seconds = time(NULL) - set_time;
				alarm(new_seconds);
				wait_alarm = false;
			}

			while (waitpid(-1, NULL, 0) > 0)
				;
		}
	}

	while (amount < original_amount)
	{
		int status;
		pid_t child_pid = waitpid(-1, &status, WNOHANG);
		if (child_pid > 0)
		{
			ended_handler(child_pid, status);
		}
	}

	for (int i = 0; i < list_len; i++)
	{
		SonProcess *son = processlist_at_index(list, i);
		int elapsed_time = (int)round(son->time_ended - son->time_created) / CLOCKS_PER_SEC;
		fprintf(csv_file, "%s,%d,%d\n", son->executable, elapsed_time, son->status);
		printf("Elapsed time process %d: %d\n", i + 1, elapsed_time);
	}

	fclose(csv_file);
	input_file_destroy(input_file);
	processlist_destroy(list);

	return 0;
}