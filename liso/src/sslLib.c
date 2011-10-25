#include "sslLib.h"


SSL_CTX *initSSL(char *crtFile, char *keyFile)
{

    SSL_CTX *ctx;
    //OpenSSL_add_all_algorithms();
    SSL_library_init();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(TLSv1_server_method());

    if(ctx == NULL) {
        logger(LogProd, "Error creating CTX object\n");
        ERR_print_errors_fp(getLogger());
        return NULL;
    }
    if ( SSL_CTX_use_certificate_file(ctx, crtFile, SSL_FILETYPE_PEM) <= 0) {
        logger(LogProd, "Error loading crtFile\n");
        ERR_print_errors_fp(getLogger());
        SSL_CTX_free(ctx);
        return NULL;
    }
    if ( SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) <= 0) {
        logger(LogProd, "Error loading keyFile\n");
        ERR_print_errors_fp(getLogger());
        SSL_CTX_free(ctx);
        return NULL;
    }

    if ( !SSL_CTX_check_private_key(ctx) ) {
        logger(LogProd, "Checking keyFile failed.\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    logger(LogProd, "SSL initialized.\n");
    return ctx;

}
