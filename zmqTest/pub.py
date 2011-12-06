#!/usr/bin/env python
import sys
import zmq
from random import choice
from string import lowercase 

context = zmq.Context()
socket = context.socket(zmq.PUB)
print "Connecting"
socket.connect("tcp://127.0.0.1:2000")
while(True):
    data="".join(choice(lowercase) for i in range(22800))
    print "Sending..."
    print data[1:5]
    print data[-5:-1]
    ret=socket.send(data)
    print ret

