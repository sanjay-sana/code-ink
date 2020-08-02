from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

ADDR = "10.1.2.2"
PORT = 8000

class RequestHandler(BaseHTTPRequestHandler):        
    def do_POST(self):
        print(self.path)
        length = int(self.headers['Content-length'])
        print(self.rfile.read(length))
        self.send_response(200, "OK")
        self.end_headers()
        self.wfile.write("serverdata")

httpd = HTTPServer((ADDR, PORT), RequestHandler)
print httpd
httpd.serve_forever()