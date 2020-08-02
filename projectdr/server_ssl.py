import socket, ssl

bindsocket = socket.socket()
bindsocket.bind(('10.1.2.2', 10023))
bindsocket.listen(5)

def deal_with_client(connstream):

   data = connstream.read()
   # null data means the client is finished with us
   while data:
	  if not do_something(connstream, data):
		 # we'll assume do_something returns False
		 # when we're finished with client
		 break
	  data = connstream.read()
   # finished with client
   connstream.close()

while True:
   newsocket, fromaddr = bindsocket.accept()
   print "connected"
   connstream = ssl.wrap_socket(newsocket,
								server_side=True,
								certfile="mycertfile",
								keyfile="mykeyfile",
								ssl_version=ssl.PROTOCOL_TLSv1)
   deal_with_client(connstream)
