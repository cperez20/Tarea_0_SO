// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>

typedef struct process{

    int pid;
    int ppid;
    int gid;
    char* status;
    bool ended;
    bool ended_before;
    int used_cpu;

} Process;