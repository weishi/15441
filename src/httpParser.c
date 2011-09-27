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
    free(req->uri);
    free(req->content);
    freeList(req->header);
    free(req);
}


enum Status httpParse(requestObj *req, char *bufPtr, ssize_t *size)
{
    if(req == NULL || req->curState == requestDone) {
        *size = 0;
        return Parsed;
    }
    ssize_t curSize = *size;
    char *buf = bufPtr;
    char *bufEnd = buf + curSize;
    char *thisPtr = buf;
    char *nextPtr;
    ssize_t parsedSize;
    while(1) {
        if(req->curState == content) {
            nextPtr = bufEnd + 1;
        } else {
            nextPtr = nextToken(thisPtr);
        }
        if(nextPtr != NULL) {
            parsedSize = (size_t)(nextPtr - thisPtr);
            httpParseLine(req, thisPtr, parsedSize, &parsedSize);
        } else {
            break;
        }
        if(req->curState == requestError) {
            break;
        }
        /* Prepare for next line */
        thisPtr = thisPtr + parsedSize;
        if(thisPtr > bufEnd) {
            break;
        }
    }
    /* clean up parsed buffer */
    if(thisPtr < bufEnd) {
        *size = bufEnd - thisPtr;
    }
    /* Set return status */
    if(req->curState == requestError) {
        return ParseError;
    } else if(req->curState == requestDone) {
        return Parsed;
    } else {
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
                uri=realloc(uri, strlen(uri) + 1);
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
        if(strcmp(line, "\r\n") == 0) {
            if(isValidRequest(req)) {
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
            char *key = malloc(lineSize);
            char *value = malloc(lineSize);
            if(sscanf(line, "%[a-zA-Z0-9-]:%[^\r\n]", key, value) != 2) {
                setRequestError(req, BAD_REQUEST);
            } else {
                key=realloc(key, strlen(key) + 1);
                value=realloc(value, strlen(value) + 1);
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
            req->content=realloc(req->content, curLength + lineSize);
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
}

char *nextToken(char *buf)
{
    char *next = strstr(buf, "\r\n");
    if(next == NULL) {
        return NULL;
    } else {
        return next + 2;
    }
}

