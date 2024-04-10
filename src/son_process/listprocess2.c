#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "listprocess2.h"
#include "process2.h"

// Definimos el inicializador de la lista, recibe el valor a guardar
ProcessList2* processlist2_init(Process value)
{
  // Pedimos memoria para la lista
  ProcessList2* list = malloc(sizeof(ProcessList2));

  // Inicializamos la lista directamente
  *list = (ProcessList2) {
    .value = value,
    .next = NULL, // como no tiene siguiente lista y es un puntero, debe partir en NULL
  };

  // Retornamos la lista
  return list;
}

// Definimos una funcion para agregar un valor a la lista
void processlist2_append(ProcessList2* list, Process value)
{
  // Aqui podemos apreciar la utilidad de esta modelacion, el código queda
  // super ordenado y legible

  // Debemos ir al final de la lista, para esto podemos utilizar un while
  ProcessList2 *last = list;
  while (last->next) {
    last = last->next;
  }

  // Creamos el nuevo nodo
  ProcessList2 *new_list = processlist2_init(value);

  // Lo agregamos como el nodo siguiente al ultimo nodo de la lista
  last->next = new_list;
}

// Definimos una funcion que nos retorne el valor en un indice de la lista
Process* processlist2_at_index(ProcessList2* list, int index)
{
  // Nos ubicamos en el indice correspondiente
  ProcessList2* node = list;
  for (size_t i = 0; i < index; i++)
  {
    node = node->next;
  }

  // Retornamos el valor
  return &node->value;
}

// Definimos el destructor de la lista
void processlist2_destroy(ProcessList2 *list)
{

  if (list == NULL)
  {
    return;
  }

  // Si hay un nodo en la sig posicion, llamamos recursivamente a la funcion
  if (list->next)
  {
    processlist2_destroy(list->next);
  }
  
  // Luego, liberamos la lista
  free(list);
}