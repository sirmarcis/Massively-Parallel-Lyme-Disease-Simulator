#include <stdlib.h>
#include <stdio.h>
#include <structs.h>

//Implementation of Custom nest_list to be utilized as a linked list of nests within ranks
//This implementation is also thread safe with the usage of a mutex lock per list
nest_list * nest_list_create(){
  nest_list *l = (nest_list *) malloc(sizeof(nest_list));
  l->count = 0;
  l->head = NULL;
  l->tail = NULL;
  pthread_mutex_init(&(l->mutex), NULL);
  return l;
}

void nest_list_free(nest_list * l){
  nestNode *li, *tmp;

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

nest * nest_list_add_element(nest_list *l, nest *li){
  pthread_mutex_lock(&(l->mutex));
  nestNode * newNode = (nestNode *) malloc(sizeof(nestNode));
  newNode->next = NULL;
  newNode->prev = l->tail;
  newNode->val = li;

  if (l->tail == NULL) {
    l->head = l->tail = newNode;
  }
  else {
    l->tail->next = newNode;
    l->tail = newNode;
  }
  l->count++;

  pthread_mutex_unlock(&(l->mutex));

  return li;
}

nest * pop_nest_left(nest_list *l){
  pthread_mutex_lock(&(l->mutex));
  // 3 or more case
  if(l->count > 2){
    //Save current head nest
    nest * temp = l->head->val;
    nestNode * temp2 = l->head;
    //Reassign the head to the current heads' next element
    l->head = temp2->next;
    //Reassign the previous of the new head to the tail
    l->head->prev = NULL;
    l->count--;
    temp2->next = NULL;
    temp2->prev = NULL;
    free(temp2);
    pthread_mutex_unlock(&(l->mutex));
    
    return temp;
  }
  else if (l->count == 2)
  {
    nest * temp = l->head->val;
    nestNode * temp2 = l->head;
    // Since only 1 element after this one, make both head and tail the same as last element
    l->head = temp2->next;
    l->tail = l->head;

    l->head->prev = NULL;

    l->count--;
    temp2->next = NULL;
    temp2->prev = NULL;
    free(temp2);
    pthread_mutex_unlock(&(l->mutex));
    return temp;
  }
  else if(l->count == 1){
    nest * temp = l->head->val;
    nestNode * temp2 = l->head;
    l->head = NULL;
    l->tail = NULL;
    l->count--;
    temp2->next = NULL;
    temp2->prev = NULL;
    free(temp2);
    pthread_mutex_unlock(&(l->mutex));
    return temp;
  }
  else{
    printf("Can't pop from an empty list");
    pthread_mutex_unlock(&(l->mutex));
    return NULL;
  }
}
