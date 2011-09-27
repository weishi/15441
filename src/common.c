#include "common.h"

void strLower(char *str)
{
    if(str != NULL) {
        int i = strlen(str) - 1;
        for(; i >= 0; i--) {
            str[i] = tolower(str[i]);
        }
    }
}
