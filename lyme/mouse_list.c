#include <stdlib.h>
#include <stdio.h>

//#include <mouse_list.h>
#include <structs.h>

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
    l->tail->next = li;
    l->tail = li;
  }
  l->count++;

  pthread_mutex_unlock(&(l->mutex));

  return li;
}

mouse * pop_mouse_left(mouse_list *l){
  pthread_mutex_lock(&(l->mutex));
  // 3 or more case
  if(l->count > 2){
    //Save current head mouse
    mouse * temp = l->head;
    //Reassign the head to the current heads' next element
    l->head = temp->next;
    //Reassign the previous of the new head to the tail
    l->head->prev = NULL;
    l->count--;
    temp->next = NULL;
    temp->prev = NULL;
    pthread_mutex_unlock(&(l->mutex));
    
    return temp;
  }
  else if (l->count == 2)
  {
    mouse * temp = l->head;
    // Since only 1 element after this one, make both head and tail the same as last element
    l->head = temp->next;
    l->tail = l->head;

    l->head->prev = NULL;

    l->count--;
    temp->next = NULL;
    temp->prev = NULL;
    pthread_mutex_unlock(&(l->mutex));
    return temp;
  }
  else if(l->count == 1){
    mouse * temp = l->head;
    l->head = NULL;
    l->tail = NULL;
    l->count--;
    temp->next = NULL;
    temp->prev = NULL;
    pthread_mutex_unlock(&(l->mutex));
    return temp;
  }
  else{
    printf("Can't pop from an empty list");
    pthread_mutex_unlock(&(l->mutex));
    return NULL;
  }
}
