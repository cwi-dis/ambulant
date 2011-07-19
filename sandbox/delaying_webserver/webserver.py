# Based on code (c) Jon Berg , turtlemeat.com
import sys
import string
import cgi
import time
import os
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import mimetypes
import optparse

delay=0

class MyHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        try:
            mimetype, _ = mimetypes.guess_type(self.path)
            path = os.path.normpath(self.path)
            path = os.curdir + os.sep + self.path
            print '**', path
            if os.path.isdir(path):
                path = os.path.join(path, 'index.html')
            print '***', path
            f = open(path, 'rb') #self.path has /test.html
            time.sleep(delay)
            self.send_response(200)
            self.send_header('Content-type',	mimetype)
            self.end_headers()
            self.wfile.write(f.read())
            f.close()
        except IOError:
            self.send_error(404,'File Not Found: %s' % self.path)
     
def main():
    global delay
    parser = optparse.OptionParser(description='Webserver with optional delay')
    parser.add_option('-d', '--delay', 
        action="store", type="int", dest="delay", default=0,
        help="Delay before serving each request (in seconds)")
    options, args = parser.parse_args()
    delay = options.delay
    
    try:
        server = HTTPServer(('', 8080), MyHandler)
        print 'started httpserver...'
        server.serve_forever()
    except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

if __name__ == '__main__':
    main()

