#include <stdlib.h>
#include <stdio.h>
#include "mouse_list.h"

/* Naive linked list implementation */

mouse_list * mouse_list_create(){
  mouse_list *l = (mouse_list *) malloc(sizeof(mouse_list));
  l->count = 0;
  l->head = NULL;
  l->tail = NULL;
  pthread_mutex_init(&(l->mutex), NULL);
  return l;
}

void mouse_list_free(mouse_list * l){
  mouse *li, *tmp;

  pthread_mutex_lock(&(l->mutex));

  if (l != NULL) {
    li = l->head;
    while (li != NULL) {
      tmp = li->next;
      free(li);
      li = tmp;
    }
  }

  pthread_mutex_unlock(&(l->mutex));
  pthread_mutex_destroy(&(l->mutex));
  free(l);
}

mouse * mouse_list_add_element(mouse_list *l, mouse *li){
  pthread_mutex_lock(&(l->mutex));
  li->next = NULL;
  li->prev = l->tail;

  if (l->tail == NULL) {
    l->head = l->tail = li;
  }
  else {
    l->tail = li;
  }
  l->count++;

  pthread_mutex_unlock(&(l->mutex));

  return li;
}

mouse * pop_mouse_left(mouse_list *l){
	pthread_mutex_lock(&(l->mutex));
	if(l->count >= 2){
		//Save current head mouse
		mouse * temp = l->head;
		//Reassign the head to the current heads' next element
		l->head = l->head->next;
		//Reassign the previous of the new head to the tail
		l->head->prev = l->tail;

		return temp;
	}
	else if(l->count == 1){
		mouse * temp = l->head;
		l->head = NULL;
		l->tail = NULL;
		return temp;
	}
	else{
		printf("Can't pop from an empty list");
		return NULL;
	}
	pthread_mutex_unlock(&(l->mutex));
}

/*int mouse_list_remove_element(mouse_list * l, void* ptr){
  int result = 0;
  mouse *li = l->head;

  pthread_mutex_lock(&(l->mutex));

  while (li != NULL) {
    if (li->value == ptr) {
      if (li->prev == NULL) {
        l->head = li->next;
      }
      else {
        li->prev->next = li->next;
      }

      if (li->next == NULL) {
        l->tail = li->prev;
      }
      else {
        li->next->prev = li->prev;
      }
      l->count--;
      free(li);
      result = 1;
      break;
    }
    li = li->next;
  }

  pthread_mutex_unlock(&(l->mutex));

  return result;
}*/

/*void mouse_list_each_element(mouse_list * l, int (*func)(mouse *)func){
  mouse *li;

  pthread_mutex_lock(&(l->mutex));

  li = l->head;
  while (li != NULL) {
    if (func(li) == 1) {
      break;
    }
    li = li->next;
  }

  pthread_mutex_unlock(&(l->mutex));
}*/