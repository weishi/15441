#include "httpParser.h"

fileMetadata prepareFile(char *uri, char *mode)
{
    char *path;
    struct stat fileStat;
    FILE *fd;
    fileMetadata fm;
    if(uri[strlen(uri)-1] == '/') {
        path = createPath(wwwFolder, uri, "index.html");
    } else {
        path = createPath(wwwFolder, uri, NULL);
    }
    if(stat(path, &fileStat) != 0) {
        return NULL;
    }
    fd = fopen(path, mode);
    if(fd == NULL) {
        return NULL;
    } else {
        fm = malloc(sizeof(fileMetadata));
        fm->fd = fd;
        fm->path = path;
        fm->type = getFileType(path);
        fm->lastMod = fileStat.st_mtime;
        fseek(fd, 0, SEEK_END);
        fm->length = ftell(fd);
        rewind(fd);
        return fm;
    }
}

char *loadFile(fileMetadata *fm)
{
    char *buffer = calloc(fm->length + 1, 1);
    fread(fm->fd, fm->length, 1, buffer);
    close(fm->fd);
    return buffer;
}

char *getFilePath(fileMetadata *fm){
    return fm->path;
}

char *getContentType(fileMetadata *fm)
{
    switch(fm->type) {
    case HTML:
        return "text/html";
    case CSS:
        return "text/css";
    case JPEG:
        return "image/jpeg";
    case PNG:
        return "image/png";
    default:
        return "other/type";
    }
}

char* getContentLength(fileMetadata *fm)
{
    char *buffer=malloc(512);
    sprintf(buffer, "%d",fm->length);
    return buffer;
}

time_t getLastMod(fileMetadata *fm)
{
    return fm->lastMod;
}

/* Private methods */

MIMEType getFileType(char *path)
{
    if(strlen(path) < 4) {
        return OTHER;
    } else {
        char *ext = strrchr(path, '.');
        if(strcmp(ext, ".html") == 0) {
            return HTML;
        }
        if(strcmp(ext, ".css") == 0) {
            return CSS;
        }
        if(strcmp(ext, ".png") == 0) {
            return PNG;
        }
        if(strcmp(ext, ".jpeg") == 0) {
            return JPEG;
        }
        return OTHER;
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





