/*
 *  This file contains the C source code for an echo server.
 *  The server supports concurrent clients with select()
 *
 *  Wei Shi <weishi@andrew.cmu.edu>
 */

#include "lisod.h"

int main(int argc, char *argv[])
{
    int portHTTP, portHTTPS;
    char *logFile;
    char *lockFile;
    char *wwwFolder;
    char *CGIFolder;
    char *crtFile;
    char *keyFile;
    selectEngine engine;
    if(argc != 9) {
        printf(USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    portHTTP = atoi(argv[1]);
    portHTTPS = atoi(argv[2]);
    logFile = argv[3];
    lockFile = argv[4];
    wwwFolder = argv[5];
    CGIFolder = argv[6];
    crtFile= argv[7];
    keyFile=argv[8];


    if(initLogger(logFile) == -1) {
        printf("Error opening logging file.\n");
        return EXIT_FAILURE;
    }
    if(initFileIO(lockFile, wwwFolder, CGIFolder) == -1) {
        printf("Error opening server root.\n");
        return EXIT_FAILURE;
    }
    logger(LogDebug, "HTTP = %d, HTTPS = %d\n"
            "logFile = %s\nlockFile = %s\n"
            "www = %s\nCGI = %s\n"
            "crtFile = %s\nkeyFile = %s\n",
           portHTTP,
           portHTTPS,
           logFile,
           lockFile,
           wwwFolder,
           CGIFolder,
           crtFile,
           keyFile);
    initEngine(&engine,
               portHTTP,
               portHTTPS,
               newConnectionHandler,
               readConnectionHandler,
               processConnectionHandler,
               writeConnectionHandler,
               closeConnectionHandler,
               crtFile,
               keyFile
              );
    return startEngine(&engine);
}
