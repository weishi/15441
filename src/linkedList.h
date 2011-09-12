#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct DLL {
    Node *head;
    int size;
    int (*compare)(void *, void *);
    void (*freeData)(void *);
} DLL;


void initList(DLL *list,
              int (*compare)(void *, void *),
              void (*freeData)(void *)
             );

void insertNode(DLL *list, void *data);

void removeNode(DLL *list, Node *target);

Node *searchList(DLL *list, void *data);

Node *getNode(DLL *list, int index);


int compareInt(void *data1, void *data2);
void freeInt(void *data);

#endif
