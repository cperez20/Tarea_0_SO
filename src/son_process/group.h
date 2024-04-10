// Tells the compiler to compile this file once
#pragma once
#include <stdbool.h>

typedef struct group{

    int start_time;
    bool active; 
    bool finished;
    int line;
    bool added_before;

} Group;