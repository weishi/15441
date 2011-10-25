#!/usr/bin/env python
#
# This script shows how to use Flask within a WSGI wrapper for CGI.
# Flask is a WSGI framework, so we need to translate CGI into WSGI.
#
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,
#          Charles Rang <rang@cs.cmu.edu>,
#          Wolfgang Richter <wolf@cs.cmu.edu>

import os, sys, socket, hashlib

# From Flask: http://flask.pocoo.org/docs/quickstart/
############### BEGIN FLASK QUICKSTART ##############
from flask import Flask, request

RootDir='/tmp'
VDir='/static'
Host='localhost'
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello World!'

@app.route('/rd/<int:port>/<obj>', method=['GET'])
def getFile():
    msg='GETRD '+obj
    response=sendReq(port,msg)
    if response.startswith('OK '):
        page=urllib.urlopen(response[3:])
        data=page.read()
        page.close()
        return response[3:]
    else:
        return 'Error'

@app.route('/rd/addfile/<int:port>/<obj>', method = ['POST'])
def addFile
    f=request.files['uploadFile']
    filename=hashlib.sha256(f).hexdigest()
    f.save(RootDir + VDir+'/'+filename)
    msg='ADDFILE '+ obj + ' ' + VDir + '/' + filename
    response=sendReq(port,msg)
    return response

def sendReq(port,msg):
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


################ END FLASK QUICKSTART ###############

# From PEP 333: http://www.python.org/dev/peps/pep-0333/
############### BEGIN WSGI WRAPPER ##############
def run_with_cgi(application):

    environ = dict(os.environ.items())
    environ['wsgi.input']        = sys.stdin
    environ['wsgi.errors']       = sys.stderr
    environ['wsgi.version']      = (1, 0)
    environ['wsgi.multithread']  = False
    environ['wsgi.multiprocess'] = True
    environ['wsgi.run_once']     = True

    if environ.get('HTTPS', 'off') in ('on', '1'):
        environ['wsgi.url_scheme'] = 'https'
    else:
        environ['wsgi.url_scheme'] = 'http'

    headers_set = []
    headers_sent = []

    def write(data):
        if not headers_set:
             raise AssertionError("write() before start_response()")

        elif not headers_sent:
             # Before the first output, send the stored headers
             status, response_headers = headers_sent[:] = headers_set
             http_version = environ.get('SERVER_PROTOCOL', 'HTTP/1.1')
             http_connection = environ.get('HTTP_CONNECTION','close')
             sys.stdout.write('%s %s\r\n' % (http_version, status))
             sys.stdout.write('Connection: %s\r\n' % (http_connection))
             for header in response_headers:
                 sys.stdout.write('%s: %s\r\n' % header)
             sys.stdout.write('\r\n')

        sys.stdout.write(data)
        sys.stdout.flush()

    def start_response(status, response_headers, exc_info=None):
        if exc_info:
            try:
                if headers_sent:
                    # Re-raise original exception if headers sent
                    raise exc_info[0], exc_info[1], exc_info[2]
            finally:
                exc_info = None     # avoid dangling circular ref
        elif headers_set:
            raise AssertionError("Headers already set!")

        headers_set[:] = [status, response_headers]
        return write

    result = application(environ, start_response)
    try:
        for data in result:
            if data:    # don't send headers until body appears
                write(data)
        if not headers_sent:
            write('')   # send headers now if body was empty
    finally:
        if hasattr(result, 'close'):
            result.close()
############### END WSGI WRAPPER ##############

if __name__ == '__main__':
    run_with_cgi(app)
