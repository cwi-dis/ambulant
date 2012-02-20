from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import os
import sys
import mimetypes
import posixpath
import thread
import urlparse
import webbrowser
import urllib

PORT=8842
DEBUG=False

try:
    modfile = __file__
except NameError:
    dirname = "."
else:
    dirname = os.path.dirname(modfile)

class WebServer(HTTPServer):
    allow_reuse_address = 1
    
    def __init__(self):
        self.tracer = None
        HTTPServer.__init__(self, ('', PORT), MyHandler)
        self.stopped = False
    
    def setTracer(self, tracer):
        self.tracer = tracer
        
    def _still_running(self):
        return not self.stopped
        
    def _run(self):
        while self._still_running():
            self.handle_request()
        self.server_close()
            
    def start(self):
        if DEBUG: print 'Starting webserver'
        thread.start_new_thread(self._run, ())
        url = "http://localhost:%d/visualize.html" % PORT
        #os.system("open %s" % url)
        webbrowser.open(url)
        
    def stop(self):
        if DEBUG: print 'Stopping webserver'
        self.stopped = True
        try:
            urllib.urlopen("http://localhost:%d/shutdown" % PORT)
        except IOError:
            pass
        
class MyHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        _, _, path, _, query, _ = urlparse.urlparse(self.path)
        if path[-1] == '/': path += 'index.html'
        data = None
        if path == '/data.json':
            if self.server.tracer:
                data = self.server.tracer.dump_json()
                mimetype = "application/json"
        else:
            try:
                filename = posixpath.split(path)[1]
                mimetype = mimetypes.guess_type(filename)[0]
                filename = os.path.join(dirname, filename)
                fp = open(filename)
                data = fp.read()
            except IOError:
                pass
        if data is None:
            self.send_error(404, "No data available")
        else:
            self.send_response(200)
            self.send_header('Content-type', mimetype)
            self.send_header('Cache-Control', 'no-cache')
            self.end_headers()
            self.wfile.write(data)
