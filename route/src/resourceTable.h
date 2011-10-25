#ifndef RESOURCETABLE_H
#define RESOURCETABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct resourceTable {
    int size;
} resourceTable;


int initResourceTable(resourceTable *, char *);


#endif
