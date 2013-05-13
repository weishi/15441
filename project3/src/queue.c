#include "queue.h"

queue *newqueue()
{
    queue *newObj = malloc(sizeof(queue));
    newObj->head = NULL;
    newObj->tail = NULL;
    newObj->size = 0;
    return newObj;
}

void enqueue(queue *qPtr, void *data)
{
    node *newNode = malloc(sizeof(node));
    newNode->data = data;
    if(qPtr->head == NULL) {
        qPtr->head = newNode;
        qPtr->tail = newNode;
    } else {
        qPtr->tail->next = newNode;
        qPtr->tail = newNode;
    }
    newNode->next = NULL;
    qPtr->size++;
}

void *dequeue(queue *qPtr)
{
    if(qPtr->head == NULL) {
        return NULL;
    }
    node *retNode = qPtr->head;
    void *data = retNode->data;
    if(qPtr->head == qPtr->tail) {
        qPtr->head = NULL;
        qPtr->tail = NULL;
    } else {
        qPtr->head = qPtr->head->next;
    }
    qPtr->size--;
    free(retNode);
    return data;
}

void *peek(queue *qPtr)
{
  if(qPtr->head == NULL) {
    return NULL;
  }
  else 
    return qPtr->head->data;
}

void mergeAtFront(queue *ins, queue *base)
{
	if(ins->head == NULL)
	  return;
	if(base->head == NULL){
	  printf("merging to an empty quue\n");
	  base->head = ins->head;
	  base->tail = ins->tail;
	  base->size = ins->size;
	  ins->head = NULL;
	  ins->tail = NULL;
	  ins->size = 0;
	  //ins = newqueue();
	}
	else {
	  printf("mergint to an un-empty queue\n");
	  ins->tail->next = base->head;
	  base->head = ins->head;
	  base->size += ins->size;
	  ins->head = NULL;
	  ins->tail = NULL;
	  ins->size = 0;
	  //ins = newqueue();
	}
}

void clearQueue(queue *qPtr){
  void* data = dequeue(qPtr);
  while(data != NULL){
    data = dequeue(qPtr);
  }
}

