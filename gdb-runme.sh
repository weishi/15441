make
gdb -ex run --args ./peer -p nodes.map -c example/${1}.chunks -f example/C.masterchunks -m 4 -i ${2}

