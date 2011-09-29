#include "httpResponder.h"

responseObj *createResponseObj()
{
    responseObj *newObj = malloc(sizeof(responseObj));
    newObj->header = malloc(sizeof(DLL));
    initList(newObj->header, compareHeaderEntry, freeHeaderEntry, NULL);
    newObj->statusLine = NULL;
    newObj->fileMeta = NULL;
    newObj->headerBuffer = NULL;
    newObj->fileBuffer = NULL;
    newObj->headerPtr = 0;
    newObj->filePtr = 0;
    newObj->maxHeaderPtr = 0;
    newObj->maxFilePtr = 0;
    newObj->close = 0;
    return newObj;
}

void freeResponseObj(responseObj *res)
{
    if(res != NULL) {
        logger(LogDebug, "Staring free Res: ");
        freeList(res->header);
        logger(LogDebug, "-Header");
        freeFileMeta(res->fileMeta);
        logger(LogDebug, "-fileMeta");
        free(res->headerBuffer);
        logger(LogDebug, "-Headerbuf");
        free(res->fileBuffer);
        logger(LogDebug, "-Filebuf");
        free(res);
        logger(LogDebug, "-Done\n");
    }
}

void buildResponseObj(responseObj *res, requestObj *req)
{
    int errorFlag = addStatusLine(res, req);
    DLL *header = res->header;
    /* Add general headers */
    char *dateStr = getHTTPDate(time(0));
    insertNode(header, newHeaderEntry("date", dateStr));
    free(dateStr);

    insertNode(header, newHeaderEntry("server", "Liso/1.0"));

    if(errorFlag == 1) {
        /* Serve Error request */
        logger(LogDebug, "to add connection close\n");
        insertNode(header, newHeaderEntry("connection", "close"));
        res->close = 1;
    } else {
        switch(req->method) {
        case GET:
            logger(LogDebug, "Load file for GET\n");
            res->fileBuffer = loadFile(res->fileMeta);
            res->maxFilePtr = res->fileMeta->length;
        case POST:
        case HEAD: {
            logger(LogDebug, "Add header for HEAD\n");
            char *valBuf;
            char *valPtr;
            //Connection
            valPtr = getValueByKey(req->header, "connection");
            if(valPtr != NULL && strcmp(valPtr, "close") == 0) {
                insertNode(header, newHeaderEntry("connection", "close"));
                res->close = 1;
            }
            //Content-length
            valBuf = getContentLength(res->fileMeta);
            insertNode(header, newHeaderEntry("content-length", valBuf));
            free(valBuf);
            //Content-type
            insertNode(header, newHeaderEntry("content-type",
                                              getContentType(res->fileMeta)));
            //Last-modified
            insertNode(header, newHeaderEntry("last-modified",
                                              getHTTPDate(getLastMod(res->fileMeta))));
        }
        break;
        default:
            break;
        }
    }
    logger(LogDebug, "Fill header...\n");
    fillHeader(res);
    printResponse(res);
}
void fillHeader(responseObj *res)
{
    static char *responseOrder[] = {
        "Connection",
        "Date",
        "Server",
        "Content-length",
        "Content-type",
        "Last-modified",
    };
    size_t bufSize = 0;
    size_t lineSize;
    unsigned int i;

    bufSize += strlen(res->statusLine);
    res->headerBuffer = malloc(bufSize + 1);
    strcpy(res->headerBuffer, res->statusLine);
    for(i = 0; i < sizeof(responseOrder) / sizeof(char *); i++) {
        char *key = responseOrder[i];
        char *value = getValueByKey(res->header, key);
        if(value != NULL) {
            lineSize = strlen(key) + strlen(": ") + strlen(value) + strlen("\r\n");
            res->headerBuffer = realloc(res->headerBuffer, bufSize + lineSize + 1);
            sprintf(res->headerBuffer + bufSize, "%s: %s\r\n", key, value);
            bufSize += lineSize;
        }
    }
    lineSize = strlen("\r\n");
    res->headerBuffer = realloc(res->headerBuffer, bufSize + lineSize + 1);
    sprintf(res->headerBuffer + bufSize, "\r\n");
    bufSize += lineSize;
    res->maxHeaderPtr = bufSize;
}

void printResponse(responseObj *res)
{
    logger(LogProd, "----Begin New Response----\n");
    logger(LogProd, "%s\n", res->headerBuffer);
    logger(LogProd, "----End New Response----\n");
}

int writeResponse(responseObj *res, char *buf, ssize_t maxSize, ssize_t *retSize)
{
    size_t hdPart = 0;
    size_t fdPart = 0;
    size_t headerPtr = res->headerPtr;
    size_t maxHeaderPtr = res->maxHeaderPtr;
    size_t filePtr = res->filePtr;
    size_t maxFilePtr = res->maxFilePtr;
    char *headerBuf = res->headerBuffer;
    char *fileBuf = res->fileBuffer;
    if(maxSize <= 0) {
        *retSize = 0;
        return 0;
    }
    logger(LogDebug, "Remain header =%d, file=%d\n", maxHeaderPtr - headerPtr, maxFilePtr - filePtr);
    if(headerPtr + maxSize <= maxHeaderPtr) {
        hdPart = maxSize;
    } else {
        hdPart = maxHeaderPtr - headerPtr;
        fdPart = maxSize - hdPart;
        if(filePtr + fdPart > maxFilePtr) {
            fdPart = maxFilePtr - filePtr;
        }
    }
    logger(LogDebug, "Dump hdPart=%d, fdPart=%d\n", hdPart, fdPart);
    memcpy(buf, headerBuf + res->headerPtr, hdPart);
    memcpy(buf + hdPart, fileBuf + res->filePtr, fdPart);
    res->headerPtr += hdPart;
    res->filePtr += fdPart;
    *retSize = hdPart + fdPart;
    return (res->headerPtr == maxHeaderPtr && res->filePtr == maxFilePtr);
}

int toClose(responseObj *res)
{
    return res->close;
}

char *getHTTPDate(time_t tmraw)
{
    char *dateStr = malloc(256);
    struct tm ctm = *gmtime(&tmraw);
    strftime(dateStr, 256, "%a, %d %b %Y %H:%M:%S %Z", &ctm);
    return dateStr;
}

int addStatusLine(responseObj *res, requestObj *req)
{
    logger(LogDebug, "To add status line\n");
    int errorFlag = 0;
    fileMetadata *fm;
    // 1.request error
    char **sl = &res->statusLine;
    *sl = "HTTP/1.1 200 OK\r\n";
    if(req->curState == requestError) {
        errorFlag = 1;
        switch((enum StatusCode)req->statusCode) {
        case BAD_REQUEST:
            *sl = "HTTP/1.1 400 BAD REQUEST\r\n";
            break;
        case NOT_FOUND:
            *sl = "HTTP/1.1 404 NOT FOUND\r\n";
            break;
        case LENGTH_REQUIRED:
            *sl = "HTTP/1.1 411 LENGTH REQUIRED\r\n";
            break;
        case INTERNAL_SERVER_ERROR:
            *sl = "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n";
            break;
        case NOT_IMPLEMENTED:
            *sl = "HTTP/1.1 501 NOT IMPLEMENTED\r\n";
            break;
        case SERVICE_UNAVAILABLE:
            *sl = "HTTP/1.1 503 SERVICE UNAVAILABLE\r\n";
            break;
        case HTTP_VERSION_NOT_SUPPORTED:
            *sl = "HTTP/1.1 505 HTTP VERSION NOT SUPPORTED\r\n";
            break;
        default:
            *sl = "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n";
            break;
        }
    } else {
        logger(LogDebug, "To parpare file\n");
        // 2.file error
        fm = prepareFile(req->uri, "r");
        if(fm == NULL) {
            logger(LogDebug, "Failed parpared file\n");
            errorFlag = 1;
            *sl = "HTTP/1.1 404 FILE NOT FOUND\r\n";
        } else {
            logger(LogDebug, "Success parpared file\n");
            res->fileMeta = fm;
        }
    }
    return errorFlag;
}



