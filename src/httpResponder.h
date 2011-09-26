#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "linkedList.h"
#include "httpParser.h"

typedef struct responseObj{
    char *statusLine;
    DLL *header;
    //File related
    FILE *file;
    char *filePath;
    char *fileType;
    int fileLength;
    char *fileLastMod;
    //Conn related
    char* headerBuffer;
    char *fileBuffer;
    int close;
}

/* Public methods */
responseObj *createResponseObj();
void freeResponseObj(responseObj*);
size_t writeResponse(responseObj *res, char *buf, size_t size);

/* Private methods */
char* getHTTPDate(time_t);
int addStatusLine(responseObj *res, requestObj *req);
int prepareFile(requestObj *req, responseObj *res);
char* getFileType(char *path);
char *createPath(char *dir, char *path, char *fileName);
#endif
