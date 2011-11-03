#ifndef LSA_H 
#define LSA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


typedef struct LSA {
    char version;
    char TTL;
    short type;
    unsigned int senderID;
    unsigned int seqNo;
    unsigned int numLink;
    unsigned int numObj;
    unsigned int **listLink;
    char ** listObj;
} LSA;

/* Constructor */
LSA *LSAfromBuffer(char *);
LSA *LSAfromLSA(LSA *);
LSA *newLSA();

int compareLSA(void *data1, void *data2);
void freeLSA(void *data);
int mapLSA(void *data);

/* Getters and Setters */


#endif
