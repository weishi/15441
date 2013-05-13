#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "sha.h"
#include "chunk.h"

static int get_numchunks(char *);

int main(int argc, char *argv[]) {
	// expect one argument atleast.
	if( argc < 2 ) {
		fprintf(stderr,"usage: %s <input-file>", argv[0]);
		exit(-1);
	}

	char *input_file = argv[1];
	FILE *fp = fopen(input_file, "rb");
	int numchunks = get_numchunks(input_file), i = 0;

	// check for sanity.
	if( numchunks < 0 || fp == NULL) {
		fprintf(stderr, "Can't stat the file %s: %s\n", input_file, strerror(errno));
		exit(-1);
	}

	// create the array. 
	uint8_t **hashes = (uint8_t **) malloc(numchunks * sizeof(uint8_t *));
	if( hashes == NULL ) {
		fprintf(stderr, "Out of memory!!!");
		exit(-1);
	}

	// allocate the individual elements.
	for(i=0;i<numchunks;i++) {
		hashes[i] = malloc((SHA1_HASH_SIZE)*sizeof(uint8_t));
		if( hashes[i] == NULL ) {
			fprintf(stderr, "Out of memory!!!");
			exit(-1);
		}
	}

	int chunks = make_chunks(fp, hashes);
	char ascii[SHA1_HASH_SIZE*2+1]; // the ascii string.
	for(i=0;i<chunks;i++) {
		hex2ascii(hashes[i], SHA1_HASH_SIZE, ascii);
		printf("%d %s\n",i,ascii);
		free(hashes[i]);
	}

	// free the memory.
	free(hashes);
	// close file.
	fclose(fp);

	// return success.
	return 0;
}

int get_numchunks(char *filename) {
	struct stat file;

	// the stat returned the file.
	if( !stat(filename,&file)) {
		// return the number of chunks for the file.
		double length = (double) file.st_size;
		return ceil(length / BT_CHUNK_SIZE);
	} else {
		// return error.
		return -1;
	}
}
