#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "httpParser.h"
#include "fileIO.h"

typedef struct responseObj {
    char *statusLine;
    DLL *header;
    fileMetadata *fileMeta;
    //Write Buffer related
    char *headerBuffer;
    char *fileBuffer;
    size_t headerPtr;
    size_t filePtr;
    size_t maxHeaderPtr;
    size_t maxFilePtr;

    int close;
} responseObj;

/* Public methods */
responseObj *createResponseObj();
void freeResponseObj(responseObj *);
int writeResponse(responseObj *, char *, ssize_t , ssize_t *);
void buildResponseObj(responseObj *, requestObj *);
int toClose(responseObj *);

/* Private methods */
void fillHeader(responseObj *);
char *getHTTPDate(time_t);
int addStatusLine(responseObj *res, requestObj *req);
#endif
