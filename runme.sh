make
valgrind --leak-check=full --track-origins=yes ./peer -p nodes.map -c example/${1}.chunks -f example/C.masterchunks -m 4 -i ${2}

