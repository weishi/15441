#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    int (*map)(void *);
    int (*copyData)(void *);
} DLL;


void initList(DLL *list,
              int (*compare)(void *, void *),
              void (*freeData)(void *),
              int (*map)(void *data),
              void *(*copyData)(void *)
             );
void freeList(DLL *list);
DLL* copyList(DLL *list);
void insertList(DLL *destList, DLL*srcList);

void insertNode(DLL *list, void *data);

void removeNode(DLL *list, Node *target);
void removeNodeAt(DLL *list, int index);

Node *searchList(DLL *list, void *data);

void *getNodeDataAt(DLL *, int );
Node *getNodeAt(DLL *, int );

void mapNode(DLL *);
void applyList(DLL *, void (*applyMe)(void *) );

int compareInt(void *data1, void *data2);
void freeInt(void *data);

#endif
