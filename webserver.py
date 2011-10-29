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
Host = 'unix14.andrew.cmu.edu'

@app.route('/')
def index():
	path = request.path[1:]
	if path == '':
		path = '/index.html'
	return redirect(url_for('static', filename=path[1:]))

@app.route('/rd/<int:p>', methods=["GET"])
def rd_getrd(p):
	obj=request.args['object']
	return getFile(p,obj)

@app.route('/rd/<int:p>/<obj>', methods=["GET"])
def rd_getrdpeer(p, obj):
	return getFile(p,obj)

@app.route('/rd/addfile/<int:p>', methods=["POST"])
def rd_addfile(p):
	f=request.files['uploadFile']
	obj=request.form['object']
	filename=saveFile(f)
	msg='ADDFILE '+ obj + ' /static/' + filename
	response=sendReq(p,msg)
	if response.startswith('OK'):
		ret='Object successfully uploaded.'
	else:
		ret='Oops! Uploading object failed.'
	return ret

def saveFile(fileHandle):
	filename=secure_filename(fileHandle.filename)
	path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
	fileHandle.save(path)
	hashVal=hashlib.sha256()
	with open(path,'r') as f:
		hashVal.update(f.read(4096))
	hashedFilename=hashVal.hexdigest()
	newPath = os.path.join(app.config['UPLOAD_FOLDER'], hashedFilename)
	shutil.move(path, newPath)
	return hashedFilename

def getFile(port,obj):
	msg='GETRD '+obj
	print obj
	response=sendReq(port,msg)
	if response.startswith('OK '):
		return send_file(urllib.urlopen(response[3:]))
	else:
		return 'Error'
def sendReq(port, msg):
	s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((Host, port))
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
