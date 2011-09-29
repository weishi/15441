#include "fileIO.h"


int initFileIO(char *lockFile, char *wwwFolder, char *CGIFolder)
{
    if(lockFile == NULL || wwwFolder == NULL || CGIFolder == NULL) {
        return -1;
    }
    _lockFile = lockFile;
    _wwwFolder = wwwFolder;
    _CGIFolder = CGIFolder;
    return 0;
}

void freeFileMeta(fileMetadata *fm)
{
    if(fm != NULL) {
        free(fm->path);
        free(fm);
    }
}

fileMetadata *prepareFile(char *uri, char *mode)
{
    char *path;
    struct stat fileStat;
    FILE *fd;
    fileMetadata *fm;
    if(uri[strlen(uri)-1] == '/') {
        path = createPath(_wwwFolder, uri, "index.html");
    } else {
        path = createPath(_wwwFolder, uri, NULL);
    }
    logger(LogDebug, "FilePath:[%s]\n", path);
    if(stat(path, &fileStat) != 0) {
        return NULL;
    }else{
        if(S_ISDIR(fileStat.st_mode)){
            free(path);
            path=createPath(_wwwFolder, uri, "/index.html");
        }
    }
    
    fd = fopen(path, mode);
    if(fd == NULL) {
        logger(LogDebug, "Failed open\n");
        return NULL;
    } else {
        logger(LogDebug, "Opened\n");
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
    fread(buffer, fm->length, 1, fm->fd);
    fclose(fm->fd);
    return buffer;
}

char *getFilePath(fileMetadata *fm)
{
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
    case GIF:
        return "image/gif";
    default:
        return "application/octet-stream";
    }
}

char *getContentLength(fileMetadata *fm)
{
    char *buffer = malloc(512);
    sprintf(buffer, "%d", fm->length);
    return buffer;
}

time_t getLastMod(fileMetadata *fm)
{
    return fm->lastMod;
}


int initLogger(char *logFile)
{
    logFD = fopen(logFile, "w");
    if(logFD == NULL) {
        return -1;
    } else {
        return 0;
    }
}

void logger(enum LogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if(level >= CUR_LOG_LEVEL) {
        vfprintf(logFD, format, args);
        fflush(logFD);
    }
    va_end(args);

}



/* Private methods */

enum MIMEType getFileType(char *path)
{
    if(strlen(path) < 4) {
        return OTHER;
    } else {
        char *ext = strrchr(path, '.');
        if(ext == NULL) {
            return OTHER;
        }
        if(strcmp(ext, ".html") == 0) {
            return HTML;
        }
        if(strcmp(ext, ".htm") == 0) {
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
        if(strcmp(ext, ".gif") == 0) {
            return GIF;
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
    if(dir != NULL) {
        strcpy(fullPath, dir);
    }
    if(path != NULL) {
        strcat(fullPath, path);
    }
    if(fileName != NULL) {
        strcat(fullPath, fileName);
    }
    return fullPath;
}





