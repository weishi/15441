#ifndef RESOURCETABLE_H
#define RESOURCETABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linkedList.h"

/* Resource Entry for DLL */

typedef struct resourceEntry {
    char *name;
    char *path;
} resourceEntry;

int compareResourceEntry(void *, void *);
void freeResourceEntry(void *);

/* Resource Table */

typedef struct resourceTable {
    DLL *table;
    char *resFile;
} resourceTable;

int initResourceTable(resourceTable *, char *);
char *getPathByName(resourceTable *, char *);
void insertResource(resourceTable *, char *, char *);
void writeResourceFile(resourceTable *);
/* Private methods */
int loadResourceTable(resourceTable *, char *);
resourceEntry *parseResourceLine(char *);
#endif
