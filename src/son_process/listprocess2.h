#pragma once
#include "process2.h"

typedef struct processlist2 {
  Process value;
  struct processlist2* next;
} ProcessList2;

ProcessList2* processlist2_init(Process value);
void processlist2_append(ProcessList2* list ,Process value );
Process* processlist2_at_index(ProcessList2* list, int index);
void processlist2_destroy(ProcessList2* list);