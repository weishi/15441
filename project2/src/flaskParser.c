#include "flaskParser.h"

int flaskParse(char *readBuf, ssize_t rSize, char *writeBuf, ssize_t *wSize, int full)
{
    int numMatch;
    char *objName = NULL;
    char *objPath = NULL;
    int length1 = 0;
    int length2 = 0;
    char *tmpBuf = calloc(rSize + 1, 1);
    memcpy(tmpBuf, readBuf, rSize);

    numMatch = sscanf(tmpBuf, "GETRD %d %ms", &length1, &objName);
    if(numMatch == 2) {
        memset(objName + length1, '\0', strlen(objName) - length1);
        printf("GET %s\n", objName);
        free(tmpBuf);
        return flaskGETResponse(objName, writeBuf, wSize);
    }

    numMatch = sscanf(tmpBuf, "ADDFILE %d %ms %d %ms", &length1, &objName, &length2, &objPath);
    if(numMatch == 4) {
        memset(objName + length1, '\0', strlen(objName) - length1);
        memset(objPath + length2, '\0', strlen(objPath) - length2);
        free(tmpBuf);
        return flaskADDResponse(objName, objPath, writeBuf, wSize);
    }

    free(tmpBuf);
    if(full) {
        printf("Request full. Close connection.");
    }
    printf("Bad request. Close connection.");
    return -1;
}

int flaskGETResponse(char *objName, char *writeBuf, ssize_t *wSize)
{
    routingInfo rInfo;
    int found = getRoutingInfo(objName, &rInfo);
    int retSize;
    if(!found) {
        retSize = snprintf(writeBuf, *wSize, "NOTFOUND 0");
    } else {
        char tmpbuf[8192];
        snprintf(tmpbuf, 8192, "http://%s:%d%s",
                 rInfo.host, rInfo.port, rInfo.path);
        retSize = snprintf(writeBuf, *wSize,
                 "OK %zu %s", strlen(tmpbuf), tmpbuf);
    }
    free(objName);
    free(rInfo.path);
    if(retSize < *wSize) {
        *wSize = retSize;
        printf("%d bytes of response buffered.\n", retSize);
        return 1; //Write succeed.
    } else {
        printf("Write buffer overflowed.[GET] Close connection.");
        return -1; //Write buffer overflowed.
    }
}


int flaskADDResponse(char *objName, char *objPath, char *writeBuf, ssize_t *wSize)
{
    insertLocalResource(objName, objPath);
    int retSize = snprintf(writeBuf, *wSize, "OK 0");
    free(objName);
    if(retSize < *wSize) {
        *wSize = retSize;
        return 1; //Write succeed.
    } else {
        printf("Write buffer overflowed.[ADDFILE] Close connection.");
        return -1; //Write buffer overflowed.
    }
}

