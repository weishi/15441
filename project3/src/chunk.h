/*
 * bt_parse.h
 *
 * Initial Author: Debabrata Dash
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 chunk processing
 *
 */

#ifndef _CHUNK_H_
#define _CHUNK_H_
#include <stdio.h>
#include <inttypes.h>

#define BT_CHUNK_SIZE (512 * 1024)

#define ascii2hex(ascii,len,buf) hex2binary((ascii),(len),(buf))
#define hex2ascii(buf,len,ascii) binary2hex((buf),(len),(ascii))

#ifdef __cplusplus
extern "C" {
#endif
  /* Returns the number of chunks created, return -1 on error */
  int make_chunks(FILE *fp, uint8_t **chunk_hashes);  

  /* returns the sha hash of the string */
  void shahash(uint8_t *chr, int len, uint8_t *target);

  /* converts a hex string to ascii */
  void binary2hex(uint8_t *buf, int len, char *ascii);

  /* converts an ascii to hex */
  void hex2binary(char *hex, int len, uint8_t*buf);
#ifdef __cplusplus
}
#endif

#endif /* _CHUNK_H_ */
