#
# Makefile for Project 2
# CCN Distributed HTTP
#
# Wei Shi <weishi@andrew.cmu.edu>
# Han Liu <hanl1@andrew.cmu.edu>


default:
	cd liso; make
	cd route; make
clean:
	cd liso; make clean
	cd route; make clean

