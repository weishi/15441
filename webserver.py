#!/usr/bin/env python

from flask import Flask, redirect, url_for, request
import sys
import os
import hashlib
import socket

app = Flask(__name__)

@app.route('/')
def index():
	return redirect(url_for('static', filename='index.html'))

@app.route('/rd/<int:p>', methods=["GET"])
def rd_getrd(p):
	#1. Figure out the <object-name> from the request
	#2. Same as rd_gerdpeer()
	obj=request.form['object']
	return getFile(p,obj)

@app.route('/rd/<int:p>/<obj>', methods=["GET"])
def rd_getrdpeer(p, obj):
    #1. Connect to the routing daemon on port p
    #2. Do GETRD <object-name> 
    #3 a) If response is OK <URL>, the open the URL
    #3 b) If response is 404, then show that content is  not available
	return getFile(p,obj)

@app.route('/rd/addfile/<int:p>', methods=["POST"])
def rd_addfile(p):
	#4. Connect to the routing daemon on port p
	#5. Do ADDFILE <object-name> <relative-path> 
	#6. Based on the response from the routing daemon display whether the object has been successfully uploaded/added or not 
	f=request.files['uploadFile']
	obj=request.form['object']
	filename=saveFile(f)
	msg='ADDFILE '+ obj + '/static/' + filename
	response=sendReq(port,msg)
	if response.startswith('OK'):
		ret='Object successfully uploaded.'
	else:
		ret='Oops! Uploading object failed.'
	return ret

def saveFile(fileHandle):
	tmpname=tempfile.mkstemp(dir='./static/')
	fileHandle.save(tmpname)
	hashVal=hashlib.sha256();
	with open(tmpname,'r') as f:
		hashVal.update(f.read(4096))
	filename=hashVal.hexdigest()
	shutil.move(tmpname,'./static/'+filename)
	return filename

def getFile(port,obj):
	msg='GETRD '+obj
	response=sendReq(port,msg)
	if response.startswith('OK '):
		page=urllib.urlopen(response[3:])
		data=page.read()
		page.close()
	return response[3:]
    else:
        return 'Error'
def sendReq(port, msg):
	s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((Host, Port))
	s.send(msg)
	response=[]
	while True:
		chunk=s.recv(1024)
		if not chunk:
			break
		response.append(chunk)
	return ''.join(response)
	
if __name__ == '__main__':
	if (len(sys.argv) > 1):
		servport = int(sys.argv[1])
		app.run(host='0.0.0.0', port=servport, threaded=True, processes=1)
	else:	
		print "Usage ./webserver <server-port> \n"
