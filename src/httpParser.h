#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
    SERVICE_UNAVAILAVLE = 503,
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

struct methodEntry methodTable[3];

typedef struct requestObj {
    enum Method method;
    char *uri;
    int version;
    DLL *header;
    char *content;
    int contentSize;
    int statusCode;
    enum State curState;
} requestObj;

/* Public methods */
requestObj *createRequestObj();
void freeRequestObj(requestObj*);
enum Status httpParse(requestObj *, char *, ssize_t *);

/* Private methods */
void nextToken(char *buf);
void strLower(char*);

#endif
