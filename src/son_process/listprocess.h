#pragma once
#include "process.h"

typedef struct processlist {
  SonProcess* value;
  struct processlist* next;
} ProcessList;

ProcessList* processlist_init(SonProcess* value);
void processlist_append(ProcessList* list ,SonProcess* value );
SonProcess* processlist_at_index(ProcessList* list, int index);
void processlist_destroy(ProcessList* list);