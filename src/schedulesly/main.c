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
int len_list;
int time_so = 0; // Tiempo sistema operativo
bool its_ci = true; // Nos indica si estabamos leyendo un CI (sino era un CE)
Process* process_active; // Proceso que actualmente esta activo

// Funcion para crear procesos
Process create_process(int pid_process, int father_pid, int NH, int gid, bool first_group_process, bool first_general_process, FILE* txt_file, int time_so, int num_linea, int arg, Group* group){

	Process process;

	if(NH != 0){
		if(first_group_process){
			gid = pid_process;
			process = (Process){.pid = pid_process, .ppid = father_pid, .gid = pid_process, .status = "WAITING", .ended = false, .ended_before = false, .used_cpu = 0};
			group->gid = pid_process;
		} else {
			process = (Process){.pid = pid_process, .ppid = father_pid, .gid = gid, .status = "WAITING", .ended = false, .ended_before = false, .used_cpu = 0};
		}
	} else {
		if(first_group_process){
			gid = pid_process;
			process = (Process){.pid = pid_process, .ppid = father_pid, .gid = pid_process, .status = "", .ended = false, .ended_before = false, .used_cpu = 0};
			group->gid = pid_process;
		} else {
			process = (Process){.pid = pid_process, .ppid = father_pid, .gid = gid, .status = "", .ended = false, .ended_before = false, .used_cpu = 0};
		}
	}
	if(first_general_process){
		list = processlist2_init(process);
	} else{
		processlist2_append(list, process);
	}
	group->num_process = group->num_process + 1;
	len_list = len_list + 1;
	fprintf(txt_file, "ENTER %d %d %d TIME %d LINE %d ARG %d\n", pid_process, father_pid, gid, time_so, num_linea, arg);	

}

bool work_group(FILE *txt_file, char **line, int len_line, Group* group, Process *process_active){

	bool group_finish = false; // Nos indica si un grupo termino de ejecutarse
	bool new_enter = true; // Nos indica si recien se entro a trabajar denuevo

	int work_units; // Unidades a trabajar en esta ejecucion
	if(group->work_units <= group->work_units_to_process ){
		int work_units = group->work_units;
	} else {
		int work_units = group->work_units_to_process;
	}

	for(int i = group->arg_en_ejecucion; i < len_line; i++){ // Partimos desde donde quedo
		if(i == len_line - 1){ // Ultimo CF
			if(group->work_units >= group->work_units_to_process){
				time_so = time_so + atoi(line[i]);
				group_finish = true; // Un grupo termina
				group->gid = 0; // pues termino el grupo
			}
			break;
		}
		// Verificamos si la entrada es CI o CE (sino sera CF que ya se maneja arriba)
		if(new_enter){
			if(its_ci && group->work_units > 0 ){
				if(group->left_time == 0){
					int work_time = atoi(line[i]);
					fprintf(txt_file, "RUN %d %d\n", process_active->pid, work_time);
					time_so = time_so + atoi(line[i]);
					group->work_units_to_process = group->work_units_to_process - work_time;
					process_active->used_cpu = process_active->used_cpu + atoi(line[i]);
				}
			}
			new_enter = false;
		}

		process_active->ended = true; // Para que termine por mientras

		// Si acaba de terminar un proceso
		for(int i = 0; i < len_list; i++){
			Process *process = processlist2_at_index(list, 0);
			if(process->ended && process->ended_before == false && process->gid == group->gid){
				fprintf(txt_file, "END %d TIME %d\n", process->pid, time_so);
				process->ended_before = true;
				process->status = "FINISHED";
				process->gid = 0;
			}
		}
		

	}

	return group_finish;

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
	int pid_process = 1;
	GroupsList *groups_list;
	GroupsList *inrow_groups_list;
	int inrow_len_groups_list = 0;
	bool first_group = true; // Para verificar si es el primer grupo en unirse a la cola
	bool first_process = true; // Para verificar si es el primer proceso que se crea
	bool group_active = false; // Me dice si hay algun grupo activo en ese momento



	printf("K líneas: %d\n", input_file->len);
	printf("qstart: %d - qdelta: %d - qmin: %d\n", atoi(input_file->lines[0][0]), atoi(input_file->lines[0][1]), atoi(input_file->lines[0][2]));

	// Rellenamos la lista de grupos
	for (int i = 1; i < input_file->len; ++i)
	{
		printf("TI: %d - CI: %d - NH: %d - CF: %d\n", atoi(input_file->lines[i][0]), atoi(input_file->lines[i][1]), atoi(input_file->lines[i][2]), atoi(input_file->lines[i][3]));
		// Unimos a los grupos
		Group group;
		group = (Group){.start_time = atoi(input_file->lines[i][0]) , .active = false, .finished = false, .line = i, .added_before = false, .work_units = qstart, .work_units_to_process = 0, .arg_en_ejecucion = 1, .num_process = 0};
		if(first_group){
			groups_list = groupslist_init(group);
			first_group = false;
		} else{
			groupslist_append(groups_list, group);
			}
	}

	first_group = true; // Reusamos la variable

	// Empezamos la ejecucion del programa
	while(true){

		bool end_program = true;


		// Revisamos todos los tiempos de los grupos
		for(int i = 0; i < input_file->len - 1; i++){
			Group* group = groupslist_at_index(groups_list, i);
			if(group->start_time <= time_so && group->added_before == false ){
				Group group_to_insert;
				group_to_insert = (Group){.start_time = group->start_time , .active = group->active, .finished = group->finished, .line = group->line, .added_before = true, .work_units = qstart, .work_units_to_process = 0, .arg_en_ejecucion = 1, .left_time = 0, .pos_original=i};
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

		for(int i = 0; i < inrow_len_groups_list; i++){
			Group* group = groupslist_at_index(inrow_groups_list, i);
			Group* original_group = groupslist_at_index(groups_list, group->pos_original);
			if(group_active){
				// Realizamos trabajo del grupo_activo
			} else if(group->finished == false){
				int len_line = 0;
				// Calculamos el largo de la linea
				for(int j = 0; j < MAX_SPLIT; j++){
					if(input_file->lines[group->line][j] != NULL){
						len_line = len_line + 1;
					} else{
						break;
					}
				}
				char **line = input_file->lines[group->line];
				int TI = atoi(line[0]);
				bool is_the_lowest = true; // Variable para saber si es el con menor tiempo de llegada
				for(int j = 0; j < inrow_len_groups_list; j++){ 
					Group* group_time_revision = groupslist_at_index(inrow_groups_list, i);
					if(group_time_revision->finished == false){
						int TI_revision = atoi(input_file->lines[group_time_revision->line][0]);
						if(TI_revision < TI){
							is_the_lowest = false;
							break;
						}
					}
				}
				if(is_the_lowest == false){ // Si no es el con menor tiempo de llegada, continuamos
					continue;
				}
				if(time_so < TI){ // Verificamos si el SO debe entrar en estado IDLE
					fprintf(txt_file, "IDLE %d\n", TI - time_so);
					time_so = TI; // Avanza hasta el tiempo del nuevo proceso 
				}
				int CI = atoi(line[1]);
				int NH = atoi(line[2]);
				int CF = atoi(line[len_line - 1]);
				time_so = time_so + TI;
				// Verificamos si es el primer proceso de todos para inicializar lista
				if(first_process){
					Process first_group_process = create_process(pid_process, 0, NH, 0, true, true, txt_file, time_so, group->line, 1, group);
					first_process = false;
				} else {
					Process first_group_process = create_process(pid_process, 0, NH, 0, true, false, txt_file, time_so, group->line, 1, group);
				}
				// Seteamos valores importantes para el reporte
				original_group->gid = group->gid;
				original_group->num_process = group->num_process;
				process_active = processlist2_at_index(list, len_list - 1); //Proceso actualmente activo
				group->work_units_to_process = CI; // Seteamos el primer tiempo a las unidades para procesar
				pid_process = pid_process + 1;
				group->active = true;
				group_active = true; // Indicamos que esta procesando
				group->finished =  work_group(txt_file,line, len_line, group, process_active);
				if(group->finished){
					original_group->gid = 0; // Por mientras
				}			


			}
		}

		// Iniciamos Reporte
		fprintf(txt_file, "REPORT START\n");
		fprintf(txt_file, "TIME %d\n", (int)(time_so));
		for(int i = 0; i < len_list; i++){
			Group* group = groupslist_at_index(groups_list, i);
			fprintf(txt_file, "GROUP %d %d\n", group->gid, group->num_process);
			for(int j = 0; j < len_list; j++){
				Process *process = processlist2_at_index(list, j);
				if(process->gid == group->gid){
					fprintf(txt_file, "PROGRAM %d %d %d %s %d\n", process->pid, process->ppid, process->gid, process->status, process->used_cpu);
				}
			}
		}
		fprintf(txt_file, "REPORT END\n");

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