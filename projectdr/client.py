import httplib

conn = httplib.HTTPConnection("10.1.2.2:8000")
conn.request("POST", "/testurl", "clientdata")
conn.send("clientdata")
response = conn.getresponse()
conn.close()

print(response.read())