#!/usr/bin/perl

# Parse a debug.h file to create a debug-text.h file.

while (<STDIN>) {
  if (/^#define ([^\s]+)\s.*DBTEXT:\s+(.*)/) {
                print "{ $1, \"$2\"},\n";
        }
}
