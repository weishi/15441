head="GET / HTTP/1.1\r\nHoSt: www.google.com\r\n\r\n"

echo "$head" | nc localhost 9999
less /tmp/lisodLog
