#include "queue.h"
#include <assert.h>

node* node_init(void) {
  node* self = (node*) malloc(sizeof(node));
  self->next = NULL;
  return self;
}

void node_end(node* self) {
  free(self);
}

void queue_init(queue* self) {
  self->size = 0;
  self->head = node_init(); // list impl always has dummy head
}

void queue_end(queue* self) {
  node* p = self->head;

  // delete everythiing in the list
  while (p != NULL) {
    node* next = p->next;
    free(p);
    p = next;
  }
}

void queue_push(queue* self, int value) {
  node* item = node_init();
  item->value = value;
  item->next = self->head->next;
  self->head->next = item;
  self->size++;
}

int queue_pop(queue* self) {
  node* p = self->head->next;
  self->head->next = self->head->next->next;
  int ret = p->value;
  free(p);
  self->size--;
  return ret;
}


