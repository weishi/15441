#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct routingTable {
    int size;
} routingTable;


int initRoutingTable(routingTable *, char *);


#endif
