#ifndef QUEUE_H
#define QUEUE_H

#include "stdlib.h"

typedef struct node {
    struct node *next;
    void *data;
} node;

typedef struct queue {
    int size;
    node *head;
    node *tail;
} queue;

queue *newqueue();
void enqueue(queue *, void *data);
void *dequeue(queue *);

#endif
