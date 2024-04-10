#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "listgroups.h"
#include "group.h"

// Definimos el inicializador de la lista, recibe el valor a guardar
GroupsList* groupslist_init(Group value)
{
  // Pedimos memoria para la lista
  GroupsList* list = malloc(sizeof(GroupsList));

  // Inicializamos la lista directamente
  *list = (GroupsList) {
    .value = value,
    .next = NULL, // como no tiene siguiente lista y es un puntero, debe partir en NULL
  };

  // Retornamos la lista
  return list;
}

// Definimos una funcion para agregar un valor a la lista
void groupslist_append(GroupsList* list, Group value)
{
  // Aqui podemos apreciar la utilidad de esta modelacion, el código queda
  // super ordenado y legible

  // Debemos ir al final de la lista, para esto podemos utilizar un while
  GroupsList *last = list;
  while (last->next) {
    last = last->next;
  }

  // Creamos el nuevo nodo
  GroupsList *new_list = groupslist_init(value);

  // Lo agregamos como el nodo siguiente al ultimo nodo de la lista
  last->next = new_list;
}

// Definimos una funcion que nos retorne el valor en un indice de la lista
Group* groupslist_at_index(GroupsList* list, int index)
{
  // Nos ubicamos en el indice correspondiente
  GroupsList* node = list;
  for (size_t i = 0; i < index; i++)
  {
    node = node->next;
  }

  // Retornamos el valor
  return &node->value;
}

// Definimos el destructor de la lista
void groupslist_destroy(GroupsList *list)
{

  if (list == NULL)
  {
    return;
  }

  // Si hay un nodo en la sig posicion, llamamos recursivamente a la funcion
  if (list->next)
  {
    groupslist_destroy(list->next);
  }
  
  // Luego, liberamos la lista
  free(list);
}