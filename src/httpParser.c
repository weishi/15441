#include "httpParser.h"

requestObj *createRequestObj(int port, char *addr)
{
    char buf[16];
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

    newObj->isCGI = -1;
    newObj->envp = malloc(sizeof(DLL));
    initList(newObj->envp, compareHeaderEntry, freeHeaderEntry, NULL);
    insertENVP(newObj, "REMOTE_ADDR", addr);
    snprintf(buf, 16, "%d", port);
    insertENVP(newObj, "SERVER_PORT", buf);
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
        freeList(req->envp);
        logger(LogDebug, "-envp");
        free(req);
        logger(LogDebug, "-Done\n");
    }
}


enum Status httpParse(requestObj *req, char *bufPtr, ssize_t *size, int full)
{
    if(req == NULL || req->curState == requestDone) {
        *size = 0;
        logger(LogDebug, "Existing ReqObj. No Parsing Needed\n");
        return Parsed;
    }
    if(req->curState == requestError) {
        *size = 0;
        logger(LogDebug, "RequestError. No Parsing Needed\n");
        return ParseError;
    }
    ssize_t curSize = *size;
    char *bufEnd = bufPtr + curSize;
    char *thisPtr = bufPtr;
    char *nextPtr;
    ssize_t parsedSize;
    logger(LogDebug, "Start parsing %d bytes\n", *size);

    while(1) {
        if(req->curState == content) {
            nextPtr = bufEnd;
        } else {
            nextPtr = nextToken(thisPtr, bufEnd);
            if(nextPtr == NULL && full) {
                /* Reject header longer than buffer size */
                setRequestError(req, BAD_REQUEST);
                break;
            } else {
                full = 0;
            }
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
        if(thisPtr >= bufEnd) {
            break;
        }
        if(req->curState == requestDone) {
            break;
        }
    }
    /* clean up parsed buffer */
    if(thisPtr < bufEnd) {
        *size = thisPtr - (bufEnd - curSize);
    }
    /* Set return status */
    if(req->curState == requestError) {
        logger(LogDebug, "Parsing result: error\n");
        return ParseError;
    } else if(req->curState == requestDone) {
        logger(LogDebug, "Parsing result: done\n");
        printRequest(req);
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
        char *lineStr = calloc(lineSize + 1, 1);
        memcpy(lineStr, line, lineSize);
        int version1, version2;
        if(sscanf(lineStr, "%s %s HTTP/%d.%d\r\n", method, uri, &version1, &version2) != 4) {
            free(lineStr);
            setRequestError(req, BAD_REQUEST);
        } else {
            free(lineStr);
            logger(LogDebug, "Parsed Status Line: Method = %s, URI = %s, Version = %d\n",
                   method, uri, version2);
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
            } else if(version1 != 1 || version2 != 1  ) {
                setRequestError(req, HTTP_VERSION_NOT_SUPPORTED);
            } else {
                uri = realloc(uri, strlen(uri) + 1);
                req->uri = uri;
                req->version = version2;
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
            char *strLine = calloc(lineSize + 1, 1);
            memcpy(strLine, line, lineSize);
            if(sscanf(strLine, "%[a-zA-Z0-9-]:%[^\r\n]", key, value) != 2) {
                setRequestError(req, BAD_REQUEST);
            } else {
                key = realloc(key, strlen(key) + 1);
                value = realloc(value, strlen(value) + 1);
                insertNode(req->header, newHeaderEntry(key, value));
            }
            free(strLine);
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
            logger(LogDebug, "Stat content length=%d, curLength=%d, lineSize=%d \n", length, curLength, lineSize);
            size_t readSize = length - curLength;
            req->content = realloc(req->content, length + 1);
            req->content[length] = '\0';
            memcpy(req->content + curLength, line, readSize);
            req->contentLength = length;
            req->curState = requestDone;
            *parsedSize = readSize;
            logger(LogDebug, "Got content %d done\n", readSize);
        } else {
            req->content = realloc(req->content, curLength + lineSize + 1);
            req->content[curLength+lineSize] = '\0';
            memcpy(req->content + curLength, line, lineSize);
            req->contentLength = curLength + lineSize;
            req->curState = content;
            logger(LogDebug, "Got content %d in middle\n", lineSize);
        }
        break;
    }
    case requestError:
    case requestDone:
    default:
        break;
    }
}

int isCGIRequest(requestObj *req)
{
    if(req->isCGI != -1) {
        return req->isCGI;
    }
    if(req->curState == requestError) {
        req->isCGI = 0;
    } else {
        char *uri = req->uri;
        char *CGIprefix = "/cgi/";
        if(strncmp(uri, CGIprefix, strlen(CGIprefix)) != 0) {
            req->isCGI = 0;
        } else {
            req->isCGI = 1;
            buildENVP(req);
        }
    }
    return req->isCGI;
}


void buildENVP(requestObj *req)
{
    char *value;
    /* Parse URI */
    char *uri=req->uri;
    char *sep = strchr(uri, '?');
    char *pathInfo = uri + strlen("/cgi");
    char *queryString = NULL;
    if(sep != NULL) {
        *sep = '\0';
        queryString = sep + 1;
    }
    req->exePath=pathInfo;
    insertENVP(req, "QUERY_STRING", queryString);
    insertENVP(req, "PATH_INFO", pathInfo);
    insertENVP(req, "REQUEST_URI", req->uri);
    
    insertENVP(req, "GATEWAY_INTERFACE", "CGI/1.1");
    insertENVP(req, "SERVER_PROTOCOL", "HTTP/1.1");
    insertENVP(req, "SERVER_SOFTWARE", "Liso/1.0");
    insertENVP(req, "SCRIPT_NAME", "/cgi");

    value = getMethodString(req->method);
    insertENVP(req, "REQUEST_METHOD", value);

    value = getValueByKey(req->header, "content-length");
    insertENVP(req, "CONTENT_LENGTH", value);
    value = getValueByKey(req->header, "content-type");
    insertENVP(req, "CONTENT_TYPE", value);
    value = getValueByKey(req->header, "accept");
    insertENVP(req, "HTTP_ACCEPT", value);
    value = getValueByKey(req->header, "referer");
    insertENVP(req, "HTTP_REFERER", value);
    value = getValueByKey(req->header, "accept-encoding");
    insertENVP(req, "HTTP_ACCEPT_ENCODING", value);
    value = getValueByKey(req->header, "accept-language");
    insertENVP(req, "HTTP_ACCEPT_LANGUAGE", value);
    value = getValueByKey(req->header, "accept-charset");
    insertENVP(req, "HTTP_ACCEPT_CHARSET", value);
    value = getValueByKey(req->header, "host");
    insertENVP(req, "HTTP_HOST", value);
    value = getValueByKey(req->header, "cookie");
    insertENVP(req, "HTTP_COOKIE", value);
    value = getValueByKey(req->header, "user-agent");
    insertENVP(req, "HTTP_USER_AGENT", value);
    value = getValueByKey(req->header, "connection");
    insertENVP(req, "HTTP_CONNECTION", value);

}

void insertENVP(requestObj *req, char *key, char *value)
{
    if(value == NULL) {
        insertNode(req->envp, newHeaderEntry(key, ""));
    } else {
        insertNode(req->envp, newHeaderEntry(key, value));
    }
}

char *getMethodString(enum Method method)
{
    char *value;
    switch(method) {
    case GET:
        value = "GET";
    case HEAD:
        value = "HEAD";
    case POST:
        value = "POST";
    default:
        value = "";
    }
    return value;

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
        break;
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

void printRequest(const requestObj *req)
{
    logger(LogProd, "----Begin New Request----\n");

    logger(LogProd, "-Method: ");
    switch(req->method) {
    case GET:
        logger(LogProd, "GET\n");
        break;
    case HEAD:
        logger(LogProd, "HEAD\n");
        break;
    case POST:
        logger(LogProd, "POST\n");
        break;
    default:
        logger(LogProd, "OTHER\n");
        break;
    }
    logger(LogProd, "-URI: %s\n", req->uri);
    logger(LogProd, "-Version: HTTP/1.%d\n", req->version);
    applyList(req->header, printHeaderEntry);
    if(req->contentLength > 0) {
        logger(LogProd, "-Content : %s\n", req->content);
    }
    logger(LogProd, "----End New Request----\n");

}






