// Tells the compiler to compile this file once
#pragma once

typedef struct process{

    int pid;
    int ppid;
    int gid;
    char* status;

} Process;