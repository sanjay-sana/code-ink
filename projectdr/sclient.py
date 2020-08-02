import httplib
from time import sleep
from threading import Thread

SERVER_IP = "10.1.2.2"
SERVER_PORT = "5000"

PROXY_IP = "10.1.3.2"
PROXY_PORT = "5005"

retry_count = 0

def send_request_to_server(conn):
	global retry_count
	conn.request("GET", "/")
	retry_count = 0
	resp = conn.getresponse()
	print resp.status, resp.reason
	data = resp.read()

def session_initiation(server_ip, server_port):
	conn = httplib.HTTPSConnection(server_ip+":"+server_port)
	while True:
		send_request_to_server(conn)
		sleep(1)
	conn.close()

def session_redirect():
	print "Initiate redirect via proxy"
	conn = httplib.HTTPSConnection(server_ip+":"+server_port)
	conn.set_tunnel(PROXY_IP, PROXY_PORT)
	conn.send("Sent Stuff")

def retry_connecting_to_server():
	global retry_count
	sleep(2)
	retry_count += 1
	try:
		print retry_count
		session_initiation(SERVER_IP, SERVER_PORT)
	except StandardError:
		pass

def connecting_to_server():
	print "Start thread"
	try:
		session_initiation(SERVER_IP, SERVER_PORT)
	except StandardError:
		while True:
			retry_connecting_to_server()
			if retry_count > 3:
				session_redirect()
				break
	print "Exit thread"

t1 = Thread(target = connecting_to_server, args = [])

t1.start()
t1.join()
