#ifndef LSA_H
#define LSA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>


#include "linkedList.h"

typedef struct LSA {
    char *src;
    char *dest;
    int srcPort;
    int destPort;
    //Payload
    uint8_t version;
    uint8_t TTL;
    uint16_t type;
    uint32_t senderID;
    uint32_t seqNo;
    uint32_t numLink;
    uint32_t numObj;
    uint32_t *listLink;
    DLL *listObj;
    //Meta
    struct timeval timestamp;
    int hasRetran;
    int isDown;
    int isExpired;
} LSA;

/* Constructor */
LSA *LSAfromBuffer(char *, ssize_t, char *, int);
LSA *LSAfromLSA(LSA *);
LSA *headerLSAfromLSA(LSA *);
LSA *newLSA(uint32_t, uint32_t);

int compareLSA(void *data1, void *data2);
void freeLSA(void *data);
void *copyLSA(void *data);

void LSAtoBuffer(LSA *, char *, ssize_t*);

void replaceLSA(LSA **, LSA *);
/* Getters and Setters */
void incLSASeq(LSA *);
void setLSADest(LSA *, char*, int);

int hasLSARetran(LSA *);
void setLSARetran(LSA *);

void setLSAAck(LSA *);
int isLSAAck(LSA *);

void setLSADown(LSA *lsa);
int isLSADown(LSA *lsa);

void decLSATTL(LSA *lsa);
uint8_t getLSATTL(LSA *lsa);

void insertLSALink(LSA *, uint32_t);
void insertLSAObj(LSA *, char *);


void printLSA(LSA *lsa);
#endif
