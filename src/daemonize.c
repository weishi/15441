#include "daemonize.h"

void signal_handler(int sig)
{
    switch(sig) {
    case SIGHUP:
        signalRestartEngine();
        break;
    case SIGTERM:
        signalExitEngine();
        break;
    default:
        break;
    }
}

int daemonize(char *lock_file)
{
    int i, lfp, pid = fork();
    char str[256] = {0};
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    setsid();

    for (i = getdtablesize(); i >= 0; i--)
        close(i);

    i = open("/dev/null", O_RDWR);
    dup(i);
    dup(i);
    umask(027);

    lfp = open(lock_file, O_RDWR | O_CREAT | O_EXCL, 0640);

    if (lfp < 0)
        exit(EXIT_FAILURE);

    if (lockf(lfp, F_TLOCK, 0) < 0)
        exit(EXIT_SUCCESS);

    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str));
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);


    return EXIT_SUCCESS;
}
