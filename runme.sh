make clean
make
#valgrind --leak-check=full --track-origins=yes ./lisod 9999 7777 /tmp/lisodLog /tmp/sth /tmp/www /tmp/CGI/cgi_script.py ./weishi.key ./weishi.crt
valgrind --leak-check=full --track-origins=yes ./routed ${1} config/node1.conf config/node1.files 10 10 10 10 

