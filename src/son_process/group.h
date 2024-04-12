// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>

typedef struct group{

    int gid;
    int start_time;
    bool active; 
    bool finished;
    int line;
    bool added_before;
    int work_units; // Unidades de trabajo que le quedan al grupo
    int work_units_to_process; // Unidades que debe procesar el grupo para terminar
    int arg_en_ejecucion; // Indica que argumento esta en ejecucion en ese momento
    int left_time; // 0 indica que ningun CI, CE o CF quedo en ejecucion
    int num_process; // Numero de procesos que tiene el grupo
    int pos_original; // Posicion del grupo en la lista original

} Group;