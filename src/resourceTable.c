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
    tRes->table= malloc(sizeof(DLL));
    tRes->resFile=resFile;
    initList(tRes->table, compareResourceEntry, freeResourceEntry, NULL, NULL);
    if(resFile == NULL) {
        return 0;
    } else {
        return loadResourceTable(tRes, resFile);
    }
}

int loadResourceTable(resourceTable *tRes, char *resFile)
{
    FILE *fp;
    char *line=NULL;
    size_t len = 0;
    fp = fopen(resFile, "r");
    if(fp == NULL) {
        printf("Error reading resource table.\n");
        return -1;
    }
    while(getline(&line, &len, fp) != -1) {
        printf("%s", line);
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
    target.path=NULL;
    Node *ref = searchList(tRes->table, &target);
    if(ref == NULL) {
        return NULL;
    } else {
        resourceEntry *ret = (resourceEntry *)(ref->data);
        return ret->path;
    }
}

void fillLSAWithObj(LSA *lsa, resourceTable *tRes){
    DLL *table=tRes->table;
    int i=0;
    while(i<table->size){
        resourceEntry *entry=getNodeDataAt(table, i);
        insertLSAObj(lsa, entry->name);
        i++;
    }
}

void insertResource(resourceTable *tRes,char *objName, char *objPath){
    resourceEntry *newObj=malloc(sizeof(resourceEntry));
    newObj->name=objName;
    newObj->path=objPath;
    insertNode(tRes->table, newObj);
    writeResourceFile(tRes);
    return;
}

void writeResourceFile(resourceTable *tRes){
    FILE *fp=fopen(tRes->resFile, "w");
    int i=0;
    for(i=0;i<tRes->table->size;i++){
        resourceEntry *entry=getNodeDataAt(tRes->table, i);
        fprintf(fp, "%s %s\n", entry->name, entry->path);
    }
    fflush(fp);
    fclose(fp);
}
