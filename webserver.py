#!/usr/bin/env python

from flask import Flask, redirect, url_for, request, send_file
from werkzeug import secure_filename
import sys
import os
import hashlib
import socket
import urllib
import tempfile
import shutil


app = Flask(__name__)
UPLOAD_FOLDER = './static/'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
Host = 'unix38.andrew.cmu.edu'

@app.route('/')
def index():
	path = request.path[1:]
	if path == '':
		path = '/index.html'
	return redirect(url_for('static', filename=path[1:]))

@app.route('/rd/<int:p>', methods=["GET"])
def rd_getrd(p):
	#1. Figure out the <object-name> from the request
	#2. Same as rd_gerdpeer()
	print request
	obj=request.args['object']
	#obj=request.form['object']
	print obj
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
	print 'addfile'
	f=request.files['uploadFile']
	obj=request.form['object']
	filename=saveFile(f)
	msg='ADDFILE '+ obj + ' /static/' + filename
	print msg
	response=sendReq(p,msg)
	if response.startswith('OK'):
		ret='Object successfully uploaded.'
	else:
		ret='Oops! Uploading object failed.'
	return ret

def saveFile(fileHandle):
	print 'saveFile'
	#tmpname=tempfile.mkstemp(dir='./static/')
	filename=secure_filename(fileHandle.filename)
	path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
	fileHandle.save(path)
	hashVal=hashlib.sha256()
	with open(path,'r') as f:
		hashVal.update(f.read(4096))
	hashedFilename=hashVal.hexdigest()
	print hashedFilename
	newPath = os.path.join(app.config['UPLOAD_FOLDER'], hashedFilename)
	print 'old path: ' + path
	print 'new path: ' + newPath
	shutil.move(path, newPath)
	print 'moved, returning'
	return hashedFilename

def getFile(port,obj):
	msg='GETRD '+obj
	print 'sending resuqest ' + msg
	response=sendReq(port,msg)
	print response
	if response.startswith('OK '):
		#print response[3:]
		return send_file(urllib.urlopen(response[3:]), as_attachment=True,
				 attachment_filename='wolf.png')
	else:
		return 'Error'
def sendReq(port, msg):
	print 'sendReq'
	print msg
	print port
	s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	print 'check'
	s.connect((Host, port))
	print 'check'
	s.send(msg)
	print 'check'
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
