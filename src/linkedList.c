#include "linkedList.h"

void initList(DLL *list,
              int (*compare)(void *, void *),
              void (*freeData)(void *),
              int (*map)(void *data))
{
    list->head = NULL;
    list->size = 0;
    list->compare = compare;
    list->freeData = freeData;
    list->map = map;
}

void insertNode(DLL *list, void *data)
{
    Node *new = malloc(sizeof(Node));
    new->data = data;

    if (list->head == NULL) {
        new->prev = NULL;
        new->next = NULL;
        list->head = new;
    } else {
        Node *ref = list->head;
        while(ref->next != NULL) {
            ref = ref->next;
        }
        ref->next = new;
        new->prev = ref;
        new->next = NULL;
    }
    list->size++;
}

void removeNode(DLL *list, Node *deadNode)
{
    Node *ref = deadNode;
    Node *head = list->head;
    if (head == NULL) {
        return;
    }

    if(ref->prev == NULL) {
        //Remove from beginning
        list->head = ref->next;
    } else if(ref->next == NULL) {
        //Remove from end
        ref->prev->next = NULL;
    } else {
        ref->prev->next = ref->next;
        ref->next->prev = ref->prev;
    }
    (*list).freeData(ref->data);
    free(ref);
    list->size--;
}

Node *searchList( DLL *list, void *target )
{
    Node *ref;
    if (list->head == NULL) {
        return NULL;
    }
    if (list->compare(list->head->data, target) != 0) {
        ref = (list->head)->next;
        while(list->head != ref) {
            if (list->compare(ref->data, target) == 0) {
                return ref;
            } else {
                ref = ref->next;
            }
        }
    } else {
        return list->head;
    }
    return NULL;

}

void *getNodeDataAt(DLL *list, int index)
{
    Node *ref = getNodeAt(list, index);
    return (ref == NULL) ? NULL : ref->data;
}

void removeNodeAt(DLL *list, int index)
{
    removeNode(list, getNodeAt(list, index));
}

Node *getNodeAt(DLL *list, int index)
{
    if(index < 0 || index >= list->size) {
        return NULL;
    } else {
        int i;
        Node *ref = list->head;
        for(i = 0; i < index; i++) {
            ref = ref->next;
        }
        return ref;
    }
}

void mapNode(DLL *list)
{
    Node *ref = list->head;
    if(list->map == NULL) {
        return;
    }
    while(ref != NULL) {
        Node *next = ref->next;
        if(!list->map(ref->data)) {
            removeNode(list, ref);
        }
        ref = next;
    }
}

int compareInt(void *data1, void *data2)
{
    return (int)((intptr_t)data1 - (intptr_t)data2);
}

void freeInt(void *data)
{
    data = data;
}
