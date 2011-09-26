#include "httpParser.h"

responseObj *createResponseObj()
{
    responseObj *newObj = malloc(sizeof(responseObj));
    newObj->header = malloc(sizeof(DLL));
    initList(newObj->header, NULL, NULL, NULL);
    newObj->statusLine = NULL;
    newObj->file = NULL;
    newObj->headerBuf = NULL;
    newObj->close = 0;
    return newObj;
}

void buildResponseObj(responseObj *res, requestObj *req)
{
    int errorFlag = addStatusLine(res, req);
    DLL *header = res->header;
    /* Add general headers */
    char *dateStr = getHTTPDate(time(0));
    insertNode(header, newHeaderEntry("date", datestr));
    free(dateStr);

    insertNode(header, newHeaderEntry("server", "Liso/1.0"));

    insertNode(header, newHeaderEntry("content-encoding", "identity"));


    if(errorFlag == 1) {
        /* Serve Error request */
        insertNode(header, newHeaderEntry("connection", "close"));
        res->close = 1;
    } else {
        switch(req->method) {
        case POST:
        case GET:
            res->fileBuffer = calloc(res->fileLength + 1, sizeof(char));
            fread(res->file, res->fileLength, 1, res->fileBuffer);
        case HEAD:
            char *valBuf;
            char *valPtr;
            //Connection
            *valPtr = getValueByKey(req, "connection");
            if(valPtr != NULL && strcmp(valPtr, "close") == 0) {
                insertNode(header, newHeaderEntry("connection", "close"));
                res->close = 1;
            }
            //Location
            valPtr = getValueByKey(req, "host");
            valBuf = malloc(strlen(res->filePath) + strlen(valPtr) + 1);
            strcpy(locationBuf, valPtr);
            strcat(locationBuf, filePath);
            insertNode(header, newHeaderEntry("location", valBuf));
            free(valBuf);
            //Content-length
            valBuf = malloc(512);
            itoa(res->fileLength, valBuf, 10);
            insertNode(header, newHeaderEntry("content-length", valBuf));
            free(valBuf);
            //Content-type
            insertNode(header, newHeaderEntry("content-type", res->fileType));
            //Last-modified
            insertNode(header, newHeaderEntry("last-modified", res->fileLastMod));
            break;
        default:
        }
        //fill up headerBuffer
        //Combine header + file buffer

    }
}

char *getHTTPDate(time_t tmraw)
{
    char *dateStr = malloc(256);
    struct tm ctm = *gmtime(&tmraw);
    strftime(datestr, sizeof(datestr), "%a, %d %b %Y %H:%M:%S %Z", &ctm);
    return dateStr;
}

int addStatusLine(responseObj *res, requestObj *req)
{
    int errorFlag = 0;
    // 1.request error
    char *sl = res->statusLine;
    sl = "HTTP/1.1 200 OK";
    if(req->curState == requestError) {
        errorFlag = 1;
        switch((StatusCode)req->statsCode) {
        case BAD_REQUEST:
            sl = "HTTP/1.1 400 BAD REQUEST";
            break;
        case NOT_FOUND:
            sl = "HTTP/1.1 404 NOT FOUND";
        case LENGTH_REQUIRED:
            sl = "HTTP/1.1 411 LENGTH REQUIRED";
            break;
        case INTERNAL_SERVER_ERROR:
            sl = "HTTP/1.1 500 INTERNAL SERVER ERROR";
            break;
        case NOT_IMPLEMENTED:
            sl = "HTTP/1.1 501 NOT IMPLEMENTED";
            break;
        case SERVICE_UNAVAILABLE:
            sl = "HTTP/1.1 503 SERVICE UNAVAILABLE";
            break;
        case HTTP_VERSION_NOT_SUPPORTED:
            sl = "HTTP/1.1 505 HTTP VERSION NOT SUPPORTED";
            break;
        default:
            sl = "HTTP/1.1 500 INTERNAL SERVER ERROR";
            break;
        }
    }
    // 2.file error
    if(prepareFile(req, res) == -1) {
        errorFlag = 1;
        sl = "HTTP/1.1 404 FILE NOT FOUND";
    }
    return errorFlag;
}


void freeResponseObj(responseObj *res)
{
    free(res);
}

int prepareFile(requestObj *req, responseObj *res)
{
    char *uri = req->uri;
    char *path;
    char *location
    struct stat fileStat;
    if(uri[strlen(uri)-1] == '/') {
        location = createPath(wwwFolder, uri, "index.html");
        path = createPath(NULL, uri, "index.html");
    } else {
        location = createPath(wwwFolder, uri, NULL);
        path = createPath(NULL, uri, NULL);
    }
    if(stat(path, &fileStat) != 0) {
        return -1;
    }
    res->file = fopen(path, "r");
    if(res->file == NULL) {
        return -1;
    } else {
        res->filePath = location;
        res->fileLastMod = getHTTPDate(fileStat.st_mtime);
        fseek(res->file, 0, SEEK_END);
        res->fileLength = ftell(res->file);
        rewind(res->file);
        res->fileType = getFileType(path);
    }
}


char *getFileType(char *path)
{
    if(strlen(path) < 4) {
        return "no/type";
    } else {
        char *ext = strrchr(path, '.');
        if(strcmp(ext, ".html") == 0) {
            return "text/html";
        }
        if(strcmp(ext, ".css") == 0) {
            return "text/css";
        }
        if(strcmp(ext, ".png") == 0) {
            return "image/png";
        }
        if(strcmp(ext, ".jpeg") == 0) {
            return "image/jpeg";
        }
        return "other/type";
    }

}

char *createPath(char *dir, char *path, char *fileName)
{
    int dirLength = (dir == NULL) ? 0 : strlen(dir);
    int pathLength = (path == NULL) ? 0 : strlen(path);
    int fileLength = (fileName == NULL) ? 0 : strlen(fileName);
    char *fullPath = malloc(dirLength + pathLength + fileLength + 1);
    strcpy(fullPath, dir);
    strcat(fullPath, path);
    strcat(fullPath, fileName);
    return fullPath;
}





