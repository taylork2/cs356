import socket
import sys 
import ctypes
import struct 

INF = 100000; #infinity 

N = 4 #number of routers 
HOST = 'localhost'
PORTS = [10001, 10002, 10003, 10004]

# Distance vector 
DISTANCES = [[0,1,4,8], 
	 		 [1,0,2,INF],
	 		 [4,2,0,1],
	 		 [8,INF,1,0]]



# Usage message 
def usage(progname, msg):
	print progname + " " + msg
	print "Usage: python " + progname + " " + "[router# (1-4)]"

def createDistanceVector(router):
	dists = []
	for i in range(1,5):
		if i == router:
			dists.append(DISTANCES[i-1])
		else:
			dists.append([INF, INF, INF, INF])

	print "Router " + str(router) + "'s Distance vector"
	print dists

	return dists

def createMessage(seq_num, router, dists):
	# convert to network byte order 
	seq_ulong = socket.htonl(seq_num)
	rou_ulong = socket.htonl(router)
	dists_flat = [d for sublist in dists for d in sublist]

	#message format (seq_num, router, flattened distance vector)
	message = struct.pack('!ii%ui' %len(dists_flat), seq_ulong, rou_ulong, *dists_flat)
	return message 

def convertMsg(message):
	d = struct.unpack('!ii16i', message)
	seq_num = socket.nltoh(d[0]);  


# Check the commandline arguments
def main(): 
	if len(sys.argv) != 2:
		usage(sys.argv[0], "Incorrect commandline arguments.")
		return 1
	elif int(sys.argv[1]) not in range(1,5):
		usage(sys.argv[0], "Port number must be 1,2,3 or 4")
		return 1

	#get the port number 
	R=int(sys.argv[1])
	PORT = PORTS[R-1]
	
	dists = createDistanceVector(R)

	# Create UDP socket 
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	except socket.error as msg:
		print "Caught socket creation exception : %s" %msg
		return 1  

	# Bind socket to port 
	serv_addr = (HOST, PORT)
	try:
		sock.bind(serv_addr)
	except socket.error as msg:
		print "Caught socket binding exception : %s" %msg
		return 1

	#set socket timeout of 1 second  
	sock.settimeout(10)

	print >> sys.stderr, 'starting up on %s port %s' %serv_addr

	seq_num = 1

	try:
		#Send distance vector to other routers 
		for i in range(1, N+1):
			if i==R:
				continue

			message = createMessage(seq_num, R, dists)

			r_addr = (HOST, PORTS[i-1])
			try:
				sent = sock.sendto(message, r_addr)
				print "Sending distance vector to router %s" %i
			except socket.error as msg: 
				print "Caught sendto exception: %s " %msg

			try:
				data, addr = sock.recvfrom(PORTS[i-1])
				print "Receiving distance vector from router %s" %i
				print data
			except socket.error as msg:
				print "Caught recvfrom exception: %s " %msg


			
	finally:
		print "Closing socket" 
		sock.close()	


main()