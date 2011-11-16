/*
 * A debugging helper library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "debug.h"

unsigned int debug = 0;

struct debug_def {
    int debug_val;
    char *debug_def;
};

/*
 * This list is auto-generated and included at compile time.
 * To add things, edit debug.h
 */

static
struct debug_def debugs[] = {
#include "debug-text.h"
    { 0, NULL } /* End of list marker */
};

int set_debug(char *arg)
{
    int i;
    if (!arg || arg[0] == '\0') {
	return -1;
    }

    if (arg[0] == '?' || !strcmp(arg, "list")) {
	fprintf(stderr,
		"Debug values and definitions\n"
		"----------------------------\n");
	for (i = 0;  debugs[i].debug_def != NULL; i++) {
	    fprintf(stderr, "%5d  %s\n", debugs[i].debug_val,
		    debugs[i].debug_def);
	}
	return -1;
    }

    if (isdigit(arg[0])) {
	debug |= atoi(arg);
    }
    return 0;
}

#ifdef _TEST_DEBUG_
int main() {
    if (set_debug("?") != -1) {
	fprintf(stderr, "set_debug(\"?\") returned wrong result code\n");
    }
    exit(0);
}

#endif

/* Packet debugging utilities */

