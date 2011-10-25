#ifndef SSLLIB_H
#define SSLLIB_H

#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "fileIO.h"

/* Public methods */
SSL_CTX* initSSL(char *, char*);

/* Private methods */


#endif
