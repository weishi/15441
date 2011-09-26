#include "httpParser.h"

methodTable = {
    {GET, "GET"},
    {HEAD, "HEAD"},
    {POST, "POST"},
};

requestObj *createRequestObj()
{
    requestObj *newObj = malloc(sizeof(requestObj));
    newObj->method = UNIMPLEMENTED;
    newObj->uri = NULL;
    newObj->version = NULL;
    newObj->header = malloc(sizeof(DLL));
    initList(newObj->header, NULL, NULL, NULL);
    newObj->content = NULL;
    newObj->contentSize = 0;
    newObj->statusCode = 0;
    newObj->curState = requestLine;
    newObj->buffer = NULL;
    newObj->bufSize = 0;
    return newObj;
}
void freeRequestObj(requestObj *req)
{
    free(uri);
    free(content);
    free(buffer);
    freeList(header);
    free(req);
}


Status httpParse(requestObj *req, char **bufPtr, size_t *size)
{
    if(req == NULL) {
        return ParseError;
    }
    size_t curSize = *size;
    char *buf = *bufPtr;
    char *bufEnd = buf + curSize;
    char *thisPtr = buf;
    char *nextPtr;
    size_t parsedSize;
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
    if(thisPtr > bufEnd) {
        *size = 0;
    } else {
        size_t newSize = bufEnd - thisPtr;
        memmove(buf, thisPtr, newSize);
        *bufPtr = realloc(buf, newSize);
        *size = newSize;
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

void httpParseLine(requestObj *req, char *line, size_t lineSize, size_t *parsedSize)
{
    switch(req->curState) {
    case requestLine:
        char *method = malloc(lineSize);
        char *uri = malloc(lineSize);
        int version;
        if(sscanf(line, "%s %s HTTP/1.%d\r\n", method, uri, version) != 3) {
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
                realloc(uri, strlen(uri) + 1);
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
        break;
    case headerLine:
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
                realloc(key, strlen(key) + 1);
                realloc(value, strlen(value) + 1);
                insertNode(req->header, newHeaderEntry(key, value));
            }
            free(key);
            free(value);
        }
        break;
    case content:
        char *lengthStr = getValueByKey(req->header, "content-length");
        int length = atoi(lengthStr);
        int curLength = req->contentLength;
        if(curLength == length) {
            req->curState = requestDone;
        }
        if(length - curLength <= size) {
            size_t readSize = length - curLength;
            req->content = realloc(req->content, length);
            memcpy(req->content + curLength, line, readSize);
            req->contentLength = length;
            req->curState = requestDone;
            *parsedSize = readSize;
        } else {
            realloc(req->content, curLength + size);
            memcpy(req->content + curLength, line, size);
            req->contentLength = curLength + size;
            req->curState = content;
        }
        break;
    case requestError:
    case requestDone:
    default:
    }
}

int isValidRequest(requestObj *req)
{
    if(req->curState == requestError) {
        return 0;
    }
    switch(req->method) {
    case POST:
        char *length = getValueByKey(req->header, "content-length");
        if(length == NULL) {
            return 0;
        } else {
            return 1;
        }
    case HEAD:
    case GET:
        char *host = getValueByKey(req->header, "host");
        if(host == NULL) {
            return 0;
        } else {
            return 1;
        }
    case UNIMPLEMENTED:
    default:
        return 0;
    }
}

void strLower(char *str)
{
    if(str != NULL) {
        int i = strlen(str) - 1;
        for(; i >= 0; i--) {
            str[i] = tolower(str[i]);
        }
    }
}

void setRequestError(requestObj *req, StatusCode code)
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

void addBuffer(requestObj *req, char *buf, size_t size)
{
    if(req->bufSize == 0) {
        req->buffer = malloc(size);
        req->bufSize = size;
        memcpy(req->buffer, buf, size);

    } else {
        size_t oldSize = req->bufSize;
        char *newBuf = malloc(size + oldSize);
        memcpy(newBuf, req->buffer, oldSize);
        memcpy(newBuf + oldSize, buf, size);
        free(req->buffer);
        req->buffer = newBuf;
        req->bufSize = size + oldSize;
    }
}
