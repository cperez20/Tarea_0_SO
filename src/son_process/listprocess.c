#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "listprocess.h"
#include "process.h"

// Definimos el inicializador de la lista, recibe el valor a guardar
ProcessList* processlist_init(SonProcess value)
{
  // Pedimos memoria para la lista
  ProcessList* list = malloc(sizeof(ProcessList));

  // Inicializamos la lista directamente
  *list = (ProcessList) {
    .value = value,
    .next = NULL, // como no tiene siguiente lista y es un puntero, debe partir en NULL
  };

  // Retornamos la lista
  return list;
}

// Definimos una funcion para agregar un valor a la lista
void processlist_append(ProcessList* list, SonProcess value)
{
  // Aqui podemos apreciar la utilidad de esta modelacion, el código queda
  // super ordenado y legible

  // Debemos ir al final de la lista, para esto podemos utilizar un while
  ProcessList *last = list;
  while (last->next) {
    last = last->next;
  }

  // Creamos el nuevo nodo
  ProcessList *new_list = processlist_init(value);

  // Lo agregamos como el nodo siguiente al ultimo nodo de la lista
  last->next = new_list;
}

// Definimos una funcion que nos retorne el valor en un indice de la lista
SonProcess* processlist_at_index(ProcessList* list, int index)
{
  // Nos ubicamos en el indice correspondiente
  ProcessList* node = list;
  for (size_t i = 0; i < index; i++)
  {
    node = node->next;
  }

  // Retornamos el valor
  return &node->value;
}

// Definimos el destructor de la lista
void processlist_destroy(ProcessList *list)
{

  if (list == NULL)
  {
    return;
  }

  // Si hay un nodo en la sig posicion, llamamos recursivamente a la funcion
  if (list->next)
  {
    processlist_destroy(list->next);
  }
  
  // Luego, liberamos la lista
  free(list);
}

