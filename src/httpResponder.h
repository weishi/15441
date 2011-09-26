#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "linkedList.h"
#include "httpParser.h"
#include "httpHeader.h"
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
}

/* Public methods */
responseObj *createResponseObj();
void freeResponseObj(responseObj *);
size_t writeResponse(responseObj *res, char *buf, size_t size);
void buildResponseObj(responseObj *, requestObj *);
int toClose(responseObj *);

/* Private methods */
void fillHeader(responseObj *);
char *getHTTPDate(time_t);
int addStatusLine(responseObj *res, requestObj *req);
#endif
