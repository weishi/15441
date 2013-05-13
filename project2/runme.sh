make
valgrind --leak-check=full --track-origins=yes ./routed ${1} config/node${1}.conf config/node${1}.files 10 15 5 15

