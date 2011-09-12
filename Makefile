################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################

CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wshadow -Wunreachable-code -g -D_FORTIFY_SOURCE=2
SOURCE=src
OBJECTS=$(SOURCE)/lisod.o $(SOURCE)/selectEngine.o $(SOURCE)/linkedList.o $(SOURCE)/httpHandler.o

default: echo_server echo_client lisod

lisod: $(OBJECTS)
	$(CC) $(CFLAGS) -o lisod $(OBJECTS)

echo_server:
	gcc echo_server.c -o echo_server -Wall -Werror


echo_client:
	gcc echo_client.c -o echo_client -Wall -Werror

clean:
	rm -f echo_server echo_client lisod
	rm $(SOURCE)/*.o
