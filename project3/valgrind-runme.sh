make
#rm -f window_monitor.log
valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./peer -p nodes.map -c example/${1}.chunks -f example/D.masterchunks -m 4 -i ${2}

