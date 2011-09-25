#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "linkedList.h"
#include "httpParser.h"

typedef struct responseObj{
    StatusCode sc;
    DLL *header;
    FILE *file;
    char* headerBuf;
}

/* Public methods */
responseObj *createResponseObj();
void freeResponseObj(requestObj*);
size_t writeResponse(responseObj *res, char *buf, size_t size);

#endif
