make clean && make
#valgrind --leak-check=full --track-origins=yes ./lisod 9999 7777 /tmp/lisodLog /tmp/sth /tmp/www /tmp/CGI/cgi_script.py ./weishi.key ./weishi.crt
valgrind --leak-check=full --track-origins=yes ./lisod 9999 7777 /tmp/lisodLog /tmp/sth /tmp/www /tmp/flask/examples/flaskr/wsgi_wrapper.py ./weishi.key ./weishi.crt
