#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkedList.h"
#include "common.h"

typedef struct headerEntry {
    char *key;
    char *value;
} headerEntry;

char *getValueByKey(DLL *, char *);
headerEntry *newHeaderEntry(char *key, char *value);


void freeHeaderEntry(void *);
int compareHeaderEntry(void *data1, void *data2);


#endif
