#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

#include "spiffy.h"

long glSrcAddr = 0;
short gsSrcPort = 0;
int giSpiffyEnabled = 0;
long glNodeID = 0;
struct sockaddr_in gsSpiffyRouter;

ssize_t spiffy_sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
	void *newbuf = NULL;
	spiffy_header s_head;

	if (0 == giSpiffyEnabled) {
		return sendto(s, msg, len, flags, to, tolen);
        }

	newbuf = (void *) malloc(sizeof(spiffy_header) + len);
	if (newbuf == NULL) { 
		errno = ENOMEM;
		return -1; 
	}
	if (to->sa_family == AF_INET) {
        	s_head.lDestAddr = ((struct sockaddr_in*)to)->sin_addr.s_addr;
        	s_head.lDestPort = ((struct sockaddr_in*)to)->sin_port;
		// printf ("Sending to %s:%hd\n", inet_ntoa(((struct sockaddr_in*)to)->sin_addr), ntohs(((struct sockaddr_in*)to)->sin_port));
	} else {
		fprintf(stderr, "spiffy_sendto:  must specify AF_INET.  FIX YOUR CODE.\n");
		errno = ENOTSUP;
		return -1;
	}
	s_head.ID = htonl(glNodeID);
	s_head.lSrcAddr = glSrcAddr;
	s_head.lSrcPort = gsSrcPort;

	memcpy(newbuf + sizeof(spiffy_header), msg, len);
	memcpy(newbuf, &s_head, sizeof(spiffy_header));
	int retVal = sendto(s, newbuf, len + sizeof(spiffy_header), flags, (struct sockaddr *) &gsSpiffyRouter, sizeof(gsSpiffyRouter));
	free(newbuf);
        if (retVal > 0) retVal -= sizeof(spiffy_header);
	return retVal;
}

int spiffy_recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *lengthPtr) {

	char *newbuf = NULL;
	spiffy_header s_head;
        socklen_t local_fromlen;
	int retVal;
	struct sockaddr_in sRecvAddr;

	*lengthPtr = sizeof(struct sockaddr_in);
        local_fromlen = sizeof(struct sockaddr_in);

	if (!giSpiffyEnabled) {
		printf("Spiffy not enabled, using normal recvfrom\n");
		return recvfrom(socket, buffer, size, flags, addr, lengthPtr);
	}

	newbuf = (char *) malloc(size + sizeof(spiffy_header));
	if (newbuf == NULL) { 
		printf("spiffy malloc failed \n");
		return -1; 
	}

	retVal = recvfrom(socket, newbuf, size + sizeof(spiffy_header), flags, (struct sockaddr *) &sRecvAddr, lengthPtr);
	if (retVal > 0) {
		memcpy(&s_head, newbuf, sizeof(spiffy_header));
		memcpy(buffer, newbuf + sizeof(spiffy_header), size);
		addr->sa_family = AF_INET;
		((struct sockaddr_in*)addr)->sin_addr.s_addr = s_head.lSrcAddr;
		((struct sockaddr_in*)addr)->sin_port = s_head.lSrcPort;
//		printf("Spiffy recvfrom %s:%d\n", inet_ntoa(((struct sockaddr_in*)addr)->sin_addr), ntohs(((struct sockaddr_in*)addr)->sin_port));
		retVal -= sizeof(spiffy_header);
	}
	if(retVal < 0) {
		printf("Error on spiffy_recvfrom. errno: %d \n", errno);
	}
	free(newbuf);
	return retVal;
}

int spiffy_init (long lNodeID, const struct sockaddr *addr, socklen_t addrlen) {

	char *cSpiffyName = NULL;
	char *cColon = NULL;

	cSpiffyName = getenv("SPIFFY_ROUTER");
	if (!cSpiffyName) {
		fprintf(stderr, "Returning from spiffy_init: no SPIFFY_ROUTER environment set.\n");
		return 1;
	}

	cColon	= strchr(cSpiffyName, ':');
	if (!cColon) {
		fprintf(stderr, "Badly formatted SPIFFY_ROUTER:  %s\n", cSpiffyName);
		return -1;
	}
	*cColon = '\0';
	gsSpiffyRouter.sin_port = htons(atoi((cColon + 1)));

	if (!inet_aton(cSpiffyName, &(gsSpiffyRouter.sin_addr))) {
	  fprintf(stderr, "Badly formatted SPIFFY_ROUTER IP:  %s\n", cSpiffyName);
	  fprintf(stderr, "Must be an IP address.  127.0.0.1:port would be a nice format.\n");
	  return -1;
	}
	giSpiffyEnabled = 1;
	glNodeID = lNodeID;

	glSrcAddr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
	gsSrcPort = ((struct sockaddr_in *)addr)->sin_port;

	fprintf(stderr, "Spiffy local stuff:  %08x:%d\n",
		glSrcAddr, ntohs(gsSrcPort));
	fprintf(stderr, "Spiffy setup complete.  %s:%d\nDelete this line after testing.\n",
		inet_ntoa(gsSpiffyRouter.sin_addr), ntohs(gsSpiffyRouter.sin_port));
	return 0;
}
