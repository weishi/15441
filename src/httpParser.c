#include "httpParser.h"

requestObj *createRequestObj()
{
    requestObj *newObj = malloc(sizeof(requestObj));
    newObj->method = UNIMPLEMENTED;
    newObj->uri = NULL;
    newObj->version = 0;
    newObj->header = malloc(sizeof(DLL));
    initList(newObj->header, compareHeaderEntry, freeHeaderEntry, NULL);
    newObj->content = NULL;
    newObj->contentLength = 0;
    newObj->statusCode = 0;
    newObj->curState = requestLine;
    return newObj;
}
void freeRequestObj(requestObj *req)
{
    if(req != NULL) {
        logger(LogDebug, "Staring free Req: ");
        free(req->uri);
        logger(LogDebug, "-uri");
        free(req->content);
        logger(LogDebug, "-content");
        freeList(req->header);
        logger(LogDebug, "-header");
        free(req);
        logger(LogDebug, "-Done\n");
    }
}


enum Status httpParse(requestObj *req, char *bufPtr, ssize_t *size)
{
    if(req == NULL || req->curState == requestDone) {
        *size = 0;
        logger(LogDebug, "Existing ReqObj. No Parsing Needed\n");
        return Parsed;
    }
    ssize_t curSize = *size;
    char *bufEnd = bufPtr + curSize;
    char *thisPtr = bufPtr;
    char *nextPtr;
    ssize_t parsedSize;
    logger(LogDebug, "Start parsing %d bytes\n", *size);
    while(1) {
        if(req->curState == content) {
            nextPtr = bufEnd + 1;
        } else {
            nextPtr = nextToken(thisPtr, bufEnd);
        }
        if(nextPtr != NULL) {
            parsedSize = (size_t)(nextPtr - thisPtr);
            httpParseLine(req, thisPtr, parsedSize, &parsedSize);
            logger(LogDebug, "One line parsed\n");
        } else {
            break;
        }
        if(req->curState == requestError ) {
            break;
        }
        /* Prepare for next line */
        thisPtr = thisPtr + parsedSize;
        if(thisPtr > bufEnd) {
            break;
        }
        if(req->curState == requestDone) {
            break;
        }
    }
    /* clean up parsed buffer */
    if(thisPtr < bufEnd) {
        *size = bufEnd - thisPtr;
    }
    /* Set return status */
    if(req->curState == requestError) {
        logger(LogDebug, "Parsing result: error\n");
        return ParseError;
    } else if(req->curState == requestDone) {
        logger(LogDebug, "Parsing result: done\n");
        return Parsed;
    } else {
        logger(LogDebug, "Parsing result: parsing\n");
        return Parsing;
    }


}

void httpParseLine(requestObj *req, char *line, ssize_t lineSize, ssize_t *parsedSize)
{
    static struct methodEntry methodTable[] = {
        {GET, "GET"},
        {HEAD, "HEAD"},
        {POST, "POST"},
    };
    switch(req->curState) {
    case requestLine: {
        char *method = malloc(lineSize + 1);
        char *uri = malloc(lineSize + 1);
        int version;
        if(sscanf(line, "%s %s HTTP/1.%d\r\n", method, uri, &version) != 3) {
            setRequestError(req, BAD_REQUEST);
        } else {
            logger(LogDebug, "Parsed Status Line: Method = %s, URI = %s, Version = %d\n",
                   method, uri, version);
            int numMethods = sizeof(methodTable) / sizeof(methodEntry);
            int i;
            for(i = 0; i < numMethods; i++) {
                if(strcmp(methodTable[i].s, method) == 0) {
                    req->method = methodTable[i].m;
                    break;
                }
            }
            if(req->method == UNIMPLEMENTED) {
                setRequestError(req, NOT_IMPLEMENTED);
            } else if(version != 1) {
                setRequestError(req, HTTP_VERSION_NOT_SUPPORTED);
            } else {
                uri = realloc(uri, strlen(uri) + 1);
                req->uri = uri;
                req->version = version;
                req->curState = headerLine;
            }
        }
        /* Clean up */
        free(method);
        if(req->curState == requestError) {
            free(uri);
        }
    }
    break;
    case headerLine: {
        logger(LogDebug, "curState: headerLine\n");
        if(lineSize == 2 && line[0] == '\r' && line[1] == '\n') {
            logger(LogDebug, "Header Close line\n");
            if(isValidRequest(req)) {
                logger(LogDebug, "isValid\n");
                if(req->method == GET || req->method == HEAD) {
                    req->curState = requestDone;
                } else if(req->method == POST) {
                    req->curState = content;
                } else {
                    req->curState = requestError;
                }
            } else {
                setRequestError(req, LENGTH_REQUIRED);
            }
        } else {
            logger(LogDebug, "KeyValue header\n");
            char *key = malloc(lineSize);
            char *value = malloc(lineSize);
            if(sscanf(line, "%[a-zA-Z0-9-]:%[^\r\n]", key, value) != 2) {
                setRequestError(req, BAD_REQUEST);
            } else {
                key = realloc(key, strlen(key) + 1);
                value = realloc(value, strlen(value) + 1);
                insertNode(req->header, newHeaderEntry(key, value));
            }
            free(key);
            free(value);
        }
    }
    break;
    case content: {
        char *lengthStr = getValueByKey(req->header, "content-length");
        int length = atoi(lengthStr);
        int curLength = req->contentLength;
        if(curLength == length) {
            req->curState = requestDone;
        }
        if(length - curLength <= lineSize) {
            size_t readSize = length - curLength;
            req->content = realloc(req->content, length);
            memcpy(req->content + curLength, line, readSize);
            req->contentLength = length;
            req->curState = requestDone;
            *parsedSize = readSize;
        } else {
            req->content = realloc(req->content, curLength + lineSize);
            memcpy(req->content + curLength, line, lineSize);
            req->contentLength = curLength + lineSize;
            req->curState = content;
        }
        break;
    }
    case requestError:
    case requestDone:
    default:
        break;
    }
}

int isValidRequest(requestObj *req)
{
    if(req->curState == requestError) {
        return 0;
    }
    switch(req->method) {
    case POST: {
        char *length = getValueByKey(req->header, "content-length");
        if(length == NULL) {
            return 0;
        } else {
            return 1;
        }
    }
    case HEAD:
    case GET: {
        char *host = getValueByKey(req->header, "host");
        if(host == NULL) {
            return 0;
        } else {
            return 1;
        }
    }
    case UNIMPLEMENTED:
    default:
        return 0;
    }
}


void setRequestError(requestObj *req, enum StatusCode code)
{
    req->curState = requestError;
    req->statusCode = (int)code;
    logger(LogProd, "Parse Error with code = %d\n", (int)code);
}

char *nextToken(char *buf, char *bufEnd)
{
    char *next = NULL;
    logger(LogDebug, "Buf content: ");
    for(; buf < bufEnd; buf++) {
        logger(LogDebug, "%c", *buf);
        if(buf[0] == '\r' && buf[1] == '\n') {
            next = buf;
            break;
        }
    }
    logger(LogDebug, "\n");

    if(next == NULL) {
        return NULL;
    } else {
        return next + 2;
    }
}

