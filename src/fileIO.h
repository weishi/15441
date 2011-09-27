#ifndef FILEIO_H 
#define FILEIO_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

char *logFile;
char *lockFile;
char *wwwFolder;
char *CGIFolder;

enum MIMEType{
    HTML,
    CSS,
    JPEG,
    PNG,
    OTHER,
};

typedef struct fileMetadata{
   FILE *fd;
   char *path;
   enum MIMEType type;
   int length;
   time_t lastMod;
} fileMetadata;




/* Public methods */
fileMetadata prepareFile(char *, char*);
char *loadFile(fileMetadata *fm);

char* getContentType(fileMetadata *fm);
char* getFilePath(fileMetadata *fm);
char* getContentLength(fileMetadata *fm);
time_t getLastMod(fileMetadata *fm);

/* Private methods */
char *createPath(char *dir, char *path, char *fileName);
enum MIMEType getFileType(char *path);

#endif
