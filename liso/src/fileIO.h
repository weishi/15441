#ifndef FILEIO_H 
#define FILEIO_H 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#define CUR_LOG_LEVEL LogDebug

char *_lockFile;
char *_wwwFolder;
char *_CGIFolder;

FILE* logFD;

enum MIMEType{
    HTML,
    CSS,
    JPEG,
    PNG,
    GIF,
    OTHER,
};

enum LogLevel{
    LogDebug=1,
    LogProd=2,
};

typedef struct fileMetadata{
   FILE *fd;
   char *path;
   enum MIMEType type;
   int length;
   time_t lastMod;
} fileMetadata;




/* Public methods */
int initFileIO(char *, char*, char *);

fileMetadata *prepareFile(char *, char*);
void freeFileMeta(fileMetadata *);
char *loadFile(fileMetadata *fm);

char* getContentType(fileMetadata *fm);
char* getFilePath(fileMetadata *fm);
char* getContentLength(fileMetadata *fm);
time_t getLastMod(fileMetadata *fm);

char *getCGIPath();
//Logging
void logger(enum LogLevel, const char *format, ...);
int initLogger(char *logFile);
FILE* getLogger();

/* Private methods */
char *createPath(char *dir, char *path, char *fileName);
enum MIMEType getFileType(char *path);

#endif
