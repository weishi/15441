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

