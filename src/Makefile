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
CFLAGS=-Wall -Werror -g
OBJECTS=lisod.o selectEngine.o linkedList.o httpHandler.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	@rm lisod
