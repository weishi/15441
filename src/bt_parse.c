/*
 * bt_parse.c
 *
 * Initial Author: Ed Bardsley <ebardsle+441@andrew.cmu.edu>
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 command line parsing.
 *
 */

#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bt_parse.h"
#include "debug.h"

static const char* const _bt_optstring = "p:c:f:m:i:d:h";

void bt_init(bt_config_t *config, int argc, char **argv) {
  bzero(config, sizeof(bt_config_t));

  strcpy(config->output_file, "output.dat");
  strcpy(config->peer_list_file, "nodes.map");
  config->argc = argc;
  config->argv = argv;
}

void bt_usage() {
  fprintf(stderr, 
	  "usage:  peer [-h] [-d <debug>] -p <peerfile> -c <chunkfile> -m <maxconn>\n"
	  "            -f <master-chunk-file> -i <identity>\n");
}

void bt_help() {
  bt_usage();
  fprintf(stderr,
	  "         -h                help (this message)\n"
	  "         -p <peerfile>     The list of all peers\n"
	  "         -c <chunkfile>    The list of chunks\n"
	  "         -m <maxconn>      Max # of downloads\n"
	  "	    -f <master-chunk> The master chunk file\n"
	  "         -i <identity>     Which peer # am I?\n"
	  );
}

bt_peer_t *bt_peer_info(const bt_config_t *config, int peer_id)
{
  assert(config != NULL);

  bt_peer_t *p;
  for (p = config->peers; p != NULL; p = p->next) {
    if (p->id == peer_id) {
      return p;
    }
  }
  return NULL;
}
 
void bt_parse_command_line(bt_config_t *config) {
  int c, old_optind;
  bt_peer_t *p;

  int argc = config->argc;
  char **argv = config->argv;

  DPRINTF(DEBUG_INIT, "bt_parse_command_line starting\n");
  old_optind = optind;
  while ((c = getopt(argc, argv, _bt_optstring)) != -1) {
    switch(c) {
    case 'h':
      bt_help();
      exit(0);
    case 'd':
      if (set_debug(optarg) == -1) {
	fprintf(stderr, "%s:  Invalid debug argument.  Use -d list  to see debugging options.\n",
		argv[0]);
	exit(-1);
      }
      break;
    case 'p': 
      strcpy(config->peer_list_file, optarg);
      break;
    case 'c':
      strcpy(config->has_chunk_file, optarg);
      break;
    case 'f':
      strcpy(config->chunk_file, optarg);
      break;
    case 'm':
      config->max_conn = atoi(optarg);
      break;
    case 'i':
      config->identity = atoi(optarg);
      break;
    default:
      bt_usage();
      exit(-1);
    }
  }

  bt_parse_peer_list(config);

  if (config->identity == 0) {
    fprintf(stderr, "bt_parse error:  Node identity must not be zero!\n");
    exit(-1);
  }
  if ((p = bt_peer_info(config, config->identity)) == NULL) {
    fprintf(stderr, "bt_parse error:  No peer information for myself (id %d)!\n",
	    config->identity);
    exit(-1);
  }
  config->myport = ntohs(p->addr.sin_port);
  assert(config->identity != 0);
  assert(strlen(config->chunk_file) != 0);
  assert(strlen(config->has_chunk_file) != 0);

  optind = old_optind;
}

void bt_parse_peer_list(bt_config_t *config) {
  FILE *f;
  bt_peer_t *node;
  char line[BT_FILENAME_LEN], hostname[BT_FILENAME_LEN];
  int nodeid, port;
  struct hostent *host;

  assert(config != NULL);
  
  f = fopen(config->peer_list_file, "r");
  assert(f != NULL);
  
  while (fgets(line, BT_FILENAME_LEN, f) != NULL) {
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s %d", &nodeid, hostname, &port) != 0);

    node = (bt_peer_t *) malloc(sizeof(bt_peer_t));
    assert(node != NULL);

    node->id = nodeid;

    host = gethostbyname(hostname);
    assert(host != NULL);
    node->addr.sin_addr.s_addr = *(in_addr_t *)host->h_addr;
    node->addr.sin_family = AF_INET;
    node->addr.sin_port = htons(port);

    node->next = config->peers;
    config->peers = node;
  }
}

void bt_dump_config(bt_config_t *config) {
  /* Print out the results of the parsing. */
  bt_peer_t *p;
  assert(config != NULL);

  printf("15-441 PROJECT 2 PEER\n\n");
  printf("chunk-file:     %s\n", config->chunk_file);
  printf("has-chunk-file: %s\n", config->has_chunk_file);
  printf("max-conn:       %d\n", config->max_conn);
  printf("peer-identity:  %d\n", config->identity);
  printf("peer-list-file: %s\n", config->peer_list_file);
  
  for (p = config->peers; p; p = p->next) 
    printf("  peer %d: %s:%d\n", p->id, inet_ntoa(p->addr.sin_addr), ntohs(p->addr.sin_port));
}
