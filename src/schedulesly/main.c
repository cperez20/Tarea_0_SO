#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <unistd.h>
#include "../son_process/process2.h"
#include "../son_process/listprocess2.h"
#include "../son_process/group.h"
#include "../son_process/listgroups.h"
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
		fprintf(txt_file, "ENTER %d %d %d TIME %d LINE %d ARG %d\n", pid_process, father_pid, gid, time_so, num_linea, arg);
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
	GroupsList *groups_list;
	GroupsList *inrow_groups_list;
	int inrow_len_groups_list = 0;
	bool first_group = true; // Para verificar si es el primer grupo en unirse a la cola
	bool first_process = true; // Para verificar si es el primer proceso que se crea



	printf("K líneas: %d\n", input_file->len);
	printf("qstart: %d - qdelta: %d - qmin: %d\n", atoi(input_file->lines[0][0]), atoi(input_file->lines[0][1]), atoi(input_file->lines[0][2]));

	// Rellenamos la lista de grupos
	for (int i = 1; i < input_file->len; ++i)
	{
		printf("TI: %d - CI: %d - NH: %d - CF: %d\n", atoi(input_file->lines[i][0]), atoi(input_file->lines[i][1]), atoi(input_file->lines[i][2]), atoi(input_file->lines[i][3]));
		// Unimos a los grupos
		Group group;
		group = (Group){.start_time = atoi(input_file->lines[i][0]) , .active = false, .finished = false, .line = i, .added_before = false};
		if(first_group){
			groups_list = groupslist_init(group);
		} else{
			groupslist_append(groups_list, group);
			}
	}

	// Empezamos la ejecucion del programa
	while(true){

		bool end_program = true;



		// Revisamos todos los tiempos de los grupos
		for(int i = 0; i < input_file->len - 1; i++){
			Group* group = groupslist_at_index(groups_list, i);
			if(group->start_time <= time_so && group->added_before == false ){
				Group group_to_insert;
				group_to_insert = (Group){.start_time = group->start_time , .active = group->active, .finished = group->finished, .line = group->line, .added_before = true};
				if(first_group){
					inrow_groups_list = groupslist_init(group_to_insert);
					first_group = false; 
				} else {
					groupslist_append(inrow_groups_list, group_to_insert);
				}
				inrow_len_groups_list = inrow_len_groups_list + 1;
				group->added_before = true; //Indicamos que ya fue añadido
			}
		}

		// Iteramos sobre los grupos en cola
		for(int i = 0; i < inrow_len_groups_list; i++){
			Group* group = groupslist_at_index(inrow_groups_list, i);
			if(group->active == false && group->finished == false){
				int len_line = sizeof(input_file->lines[group->line])/sizeof(input_file->lines[group->line][0]);
				char **line = input_file->lines[group->line];
				// Para acceder a los valores de esa linea la referenciamos directamente
				int TI = atoi(line[0]);
				int CI = atoi(line[1]);
				int NH = atoi(line[2]);
				int CF = atoi(line[len_line - 1]);
				time_so = time_so + TI;
				// Verificamos si es el primer proceso de todos para inicializar lista
				if(first_process){
					Process first_group_process = create_process(pid_so, 0, NH, 0, true, true, txt_file, time_so, i, 1);
					first_process = false;
				} else {
					Process first_group_process = create_process(pid_so, 0, NH, 0, true, false, txt_file, time_so, i, 1);
				}
				group->finished = true; // Para que termine por mientras
			}
		}

		// Verificamos si se debe terminar el programa
		if(inrow_len_groups_list == input_file->len - 1){
			for(int i = 0; i < inrow_len_groups_list; i++){
				Group* group = groupslist_at_index(inrow_groups_list, i);
				if(group->finished == false){
					end_program = false; // No se termina el programa si un hijo no ha terminado
				}
			}
		}
		if(end_program){
			break;
		}


	}

	groupslist_destroy(groups_list);
	groupslist_destroy(inrow_groups_list);
	input_file_destroy(input_file);
	fclose(txt_file);
}