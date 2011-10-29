#include "resourceTable.h"



int compareResourceEntry(void *data1, void *data2)
{
    resourceEntry *re1 = (resourceEntry *)data1;
    resourceEntry *re2 = (resourceEntry *)data2;
    return strcmp(re1->name, re2->name);
}

void freeResourceEntry(void *data)
{
    resourceEntry *re = (resourceEntry *)data;
    free(re->name);
    free(re->path);
}


/* Resource Table */

int initResourceTable(resourceTable *tRes, char *resFile)
{
    initList(tRes->table, compareResourceEntry, freeResourceEntry, NULL);
    if(resFile == NULL) {
        return 0;
    } else {
        return loadResourceTable(tRes, resFile);
    }
}

int loadResourceTable(resourceTable *tRes, char *resFile)
{
    FILE *fp;
    char *line;
    size_t len = 0;
    fp = fopen(resFile, "r");
    if(fp == NULL) {
        printf("Error reading resource table.\n");
        return -1;
    }
    while(getline(&line, &len, fp) != 1) {
        resourceEntry *re = parseResourceLine(line);
        insertNode(tRes->table, re);
    }

    if(line != NULL) {
        free(line);
    }
    fclose(fp);
    return 0;
}

resourceEntry *parseResourceLine(char *line)
{
    int numMatch;
    resourceEntry *newObj;
    char *name = NULL;
    char *path = NULL;

    numMatch = sscanf(line, "%ms %ms", &name, &path);
    if(numMatch != 2) {
        if(name != NULL) free(name);
        if(path != NULL) free(path);

        return NULL;
    }

    newObj = malloc(sizeof(resourceEntry));
    newObj->name = name;
    newObj->path = path;
    return newObj;
}

char *getPathByName(resourceTable *tRes, char *objName)
{
    resourceEntry target;
    target.name = objName;
    Node *ref = searchList(tRes->table, &target);
    if(ref == NULL) {
        return NULL;
    } else {
        resourceEntry *ret = (resourceEntry *)(ref->data);
        return ret->path;
    }
}

void insertResource(resourceTable *tRes,char *objName, char *objPath){
    resourceEntry *newObj=malloc(sizeof(resourceEntry));
    newObj->name=objName;
    newObj->path=objPath;
    insertNode(tRes->table, newObj);
    return;
}
