#!/usr/bin/env python
import sys
import zmq
context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:3000");
filter = ""
socket.setsockopt(zmq.SUBSCRIBE, filter)
while(True):
    string = socket.recv()
    print len(string)
    print string[1:5]
    print string[-5:-1]


