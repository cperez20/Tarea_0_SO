#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <unistd.h>
#include "../son_process/process2.h"
#include "../son_process/listprocess2.h"
#include <stdbool.h>
#include <time.h>

ProcessList2 *list;

// Funcion para crear procesos
Process create_process(pid_t pid_so, int father_pid, int NH, int gid, bool first_group_process, bool first_general_process, FILE* txt_file, int time_so, int num_linea, int arg){

	Process process;

	pid_t pid = fork();

	if (pid < 0) // Si fork devuelve un numero negativo, hubo un error creando al hijo
	{
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) // Si devuelve 0 es porque estamos en el proceso hijo
	{
		// Logica dentro del hijo
	}
	else
	{
		int pid_process = (int)(pid - pid_so);
		if(NH != 0){
			if(first_group_process){
				gid = pid_process;
				process = (Process){.pid = pid_process, .ppid = father_pid, .gid = pid_process, .status = "WAITING"};
			} else {
				process = (Process){.pid = pid_process, .ppid = father_pid, .gid = gid, .status = "WAITING"};
			}
		}
		if(first_general_process){
			list = processlist2_init(process);
		} else{
			processlist2_append(list, process);
		}
		fprintf(txt_file, "ENTER %d %d %d TIME %d LINE %d ARG %d", pid_process, father_pid, gid, time_so, num_linea, arg);
	}

}

int main(int argc, char const *argv[])
{
	/*Lectura del input: ./testsT0/P2/inputs/in01.txt */
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);
	char *output_file = (char *)argv[2];
	FILE *txt_file = fopen(output_file, "w");
	int qstart = atoi(input_file->lines[0][0]);
	int qdelta = atoi(input_file->lines[0][1]);
	int qmin = atoi(input_file->lines[0][2]);
	pid_t pid_so = getpid();
	int time_so = 0;

	printf("K líneas: %d\n", input_file->len);
	printf("qstart: %d - qdelta: %d - qmin: %d\n", atoi(input_file->lines[0][0]), atoi(input_file->lines[0][1]), atoi(input_file->lines[0][2]));
	for (int i = 1; i < input_file->len; ++i)
	{
		printf("TI: %d - CI: %d - NH: %d - CF: %d\n", atoi(input_file->lines[i][0]), atoi(input_file->lines[i][1]), atoi(input_file->lines[i][2]), atoi(input_file->lines[i][3]));
		// Encontramos el largo de la linea del input para poder iterar en este
		int len_line = sizeof(input_file->lines[i])/sizeof(input_file->lines[i][0]);
		// Para acceder a los valores de esa linea la referenciamos directamente
		char **line = input_file->lines[i];
		int TI = atoi(line[0]);
		int CI = atoi(line[1]);
		int NH = atoi(line[2]);
		int CF = atoi(line[len_line - 1]);
		time_so = time_so + TI;
		// Verificamos si es el primer proceso de todos para inicializar lista
		if(i == 1){
			Process first_group_process = create_process(pid_so, 0, NH, 0, true, true, txt_file, time_so, i, 1);
		} else {
			Process first_group_process = create_process(pid_so, 0, NH, 0, true, false, txt_file, time_so, i, 1);
		}

		// Empezamos a leer los hijos
		int j = 3; // Para leer el resto de los valores de la linea
		while(j < len_line){
			int CI_subprocess = atoi(line[j]);
			int NH_subprocess = atoi(line[j + 1]);
		}
	}

	input_file_destroy(input_file);
	fclose(txt_file);
}