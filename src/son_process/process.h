// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>
#include <time.h>

typedef struct son_process {

    int pid;
    char* executable;
    bool ended;
    time_t time_created;
    time_t time_ended;
    int status;

} SonProcess ;