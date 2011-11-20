#include "queue.h"

queue *newqueue()
{
    queue *newObj = malloc(sizeof(queue));
    newObj->head = NULL;
    newObj->tail = NULL;
    newObj->size = 0;
    return newObj;
}

void enqueue(queue *qPtr, node *data)
{
    if(qPtr->head == NULL) {
        qPtr->head = data;
        qPtr->tail = data;
    } else {
        qPtr->tail->next = data;
        qPtr->tail = data;
    }
    data->next = NULL;
    qPtr->size++;
}

node *dequeue(queue *qPtr)
{
    if(qPtr->head == NULL) {
        return NULL;
    }
    node *retNode = qPtr->head;
    if(qPtr->head == qPtr->tail) {
        qPtr->head = NULL;
        qPtr->tail = NULL;
    } else {
        qPtr->head = qPtr->head->next;
    }
    qPtr->size--;
    return retNode;

}

