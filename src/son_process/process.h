// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>
#include <time.h>

typedef struct son_process {

    int pid;
    char* executable;
    double time_created;
    double time_ended;
    int status;

} SonProcess ;