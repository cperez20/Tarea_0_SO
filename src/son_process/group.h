// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>

typedef struct group{

    int start_time;
    bool active; 
    bool finished;
    int line;
    bool added_before;
    int work_units; // Unidades de trabajo que le quedan al grupo
    int work_units_to_process; // Unidades que debe procesar el grupo para terminar
    int arg_en_ejecucion // Indica que argumento esta en ejecucion en ese momento

} Group;