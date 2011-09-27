#include "httpHeader.h"

headerEntry newHeaderEntry(char *key, char *value)
{
    if(key == NULL) {
        return NULL;
    }
    headerEntry *hd = malloc(sizeof(headerEntry));
    char *thisKey = malloc(strlen(key) + 1);
    char *thisValue = NULL;
    strcpy(thisKey, key);
    if(value != NULL) {
        thisValue = malloc(strlen(value) + 1);
        strcpy(thisValue, value);
    }
    hd->key = strLower(thisKey);
    hd->value = thisValue;
    return hd;
}

void freeHeaderEntry(headerEntry *hd)
{
    if(hd != NULL) {
        free(hd->key);
        free(hd->value);
        free(hd);
    }
}

char *getValueByKey(DLL *headerList, char *key)
{
    headerEntry *target = newHeaderEntry(key, NULL);
    headerEntry *result = searchList(headerList, target);
    freeHeaderEntry(target);

    if(result == NULL) {
        return NULL;
    } else {
        return result->value;
    }
}

int compareHeaderEntry(void *data1, void *data2)
{
    headerEntry *h1 = (headerEntry)data1;
    headerEntry *h2 = (headerEntry)data2;
    return strcmp(h1->key, h2->key);
}

