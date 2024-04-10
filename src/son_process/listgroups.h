#pragma once
#include "group.h"

typedef struct groupslist {
  Group value;
  struct groupslist* next;
} GroupsList;

GroupsList* groupslist_init(Group value);
void groupslist_append(GroupsList* list ,Group value );
Group* groupslist_at_index(GroupsList* list, int index);
void groupslist_destroy(GroupsList* list);