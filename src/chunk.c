#include "sha.h"
#include "chunk.h"
#include <ctype.h>
#include <assert.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memset

/**
 * fp -- the file pointer you want to chunkify.
 * chunk_hashes -- The chunks are stored at this locations.
 */
/* Returns the number of chunks created */
int make_chunks(FILE *fp, uint8_t *chunk_hashes[]) {
	//allocate a big buffer for the chunks from the file.
	uint8_t *buffer = (uint8_t *) malloc(BT_CHUNK_SIZE);
	int numchunks = 0;
	int numbytes = 0;

	// read the bytes from the file and fill in the chunk_hashes
	while((numbytes = fread(buffer, sizeof(uint8_t), BT_CHUNK_SIZE, fp)) > 0 ) {
		shahash(buffer, numbytes, chunk_hashes[numchunks++]);
	}

	return numchunks;
}

/**
 * str -- is the data you want to hash
 * len -- the length of the data you want to hash.
 * hash -- the target where the function stores the hash. The length of the
 *         array should be SHA1_HASH_SIZE.
 */
void shahash(uint8_t *str, int len, uint8_t *hash) {
	SHA1Context sha;

	// init the context.
	SHA1Init(&sha);

	// update the hashes.
	// this can be used multiple times to add more
	// data to hash.
	SHA1Update(&sha, str, len);

	// finalize the hash and store in the target.
	SHA1Final(&sha, hash);

	// A little bit of paranoia.
	memset(&sha, 0, sizeof(sha));
}

/**
 * converts the binary char string str to ascii format. the length of 
 * ascii should be 2 times that of str
 */
void binary2hex(uint8_t *buf, int len, char *hex) {
	int i=0;
	for(i=0;i<len;i++) {
		sprintf(hex+(i*2), "%.2x", buf[i]);
	}
	hex[len*2] = 0;
}
  
/**
 *Ascii to hex conversion routine
 */
static uint8_t _hex2binary(char hex)
{
     hex = toupper(hex);
     uint8_t c = ((hex <= '9') ? (hex - '0') : (hex - ('A' - 0x0A)));
     return c;
}

/**
 * converts the ascii character string in "ascii" to binary string in "buf"
 * the length of buf should be atleast len / 2
 */
void hex2binary(char *hex, int len, uint8_t*buf) {
	int i = 0;
	for(i=0;i<len;i+=2) {
		buf[i/2] = 	_hex2binary(hex[i]) << 4 
				| _hex2binary(hex[i+1]);
	}
}

#ifdef _TEST_CHUNK_C_
int main(int argc, char *argv[]) {
	uint8_t *test = "dash";
	uint8_t hash[SHA1_HASH_SIZE], hash1[SHA1_HASH_SIZE];
	char ascii[SHA1_HASH_SIZE*2+1];

	shahash(test,4,hash);
	
	binary2hex(hash,SHA1_HASH_SIZE,ascii);

	printf("%s\n",ascii);

	assert(strlen(ascii) == 40);

	hex2binary(ascii, strlen(ascii), hash1);

	binary2hex(hash1,SHA1_HASH_SIZE,ascii);

	printf("%s\n",ascii);
}
#endif //_TEST_CHUNK_C_
