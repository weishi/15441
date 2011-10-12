#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "common.h"
#include "fileIO.h"
#include "linkedList.h"
#include "httpHeader.h"

enum State {
    requestLine,
    headerLine,
    content,
    requestError,
    requestDone,
};

enum Status {
    Parsing,
    Parsed,
    ParseError,
};

enum StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    LENGTH_REQUIRED = 411,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503,
    HTTP_VERSION_NOT_SUPPORTED = 505,
};

enum Method {
    GET,
    HEAD,
    POST,
    UNIMPLEMENTED,
};

struct methodEntry {
    enum Method m;
    char *s;
} methodEntry;


typedef struct requestObj {
    enum Method method;
    char *uri;
    int version;
    DLL *header;
    char *content;
    int contentLength;
    int statusCode;
    enum State curState;
    int isCGI;
    char *exePath;
    DLL *envp;
} requestObj;

/* Public methods */
requestObj *createRequestObj(int,char *);
void freeRequestObj(requestObj*);
enum Status httpParse(requestObj *, char *, ssize_t *, int);
int isCGIRequest(requestObj*);

/* Private methods */
void httpParseLine(requestObj *, char *, ssize_t , ssize_t *);
void setRequestError(requestObj *, enum StatusCode );
int isValidRequest(requestObj *);
char* nextToken(char *, char *);
void printRequest(const requestObj *);
void buildENVP(requestObj *);
void insertENVP(requestObj *, char*, char*);
char *getMethodString(enum Method);
#endif
