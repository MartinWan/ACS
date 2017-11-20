#ifndef QUEUE_H
#define QUEUE_H
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

typedef struct node {
  int value;
  struct node* next;
} node;

node* node_init(void);


typedef struct queue {
  node* head;
  int size;
} queue;

void queue_init(queue* self);
void queue_push(queue* self, int value);
int queue_pop(queue* self);
void queue_end(queue* self);

#endif
