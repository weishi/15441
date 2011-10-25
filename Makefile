#
# Makefile for Project 2
# CCN
#
# Wei Shi <weishi@andrew.cmu.edu>
# Han Liu <hanl1@andrew.cmu.edu>


default:
	cd liso; make
	cd route; make
clean:
	rm -f lisod
	rm -f routed
	rm liso/*.o
	rm route/*.o

