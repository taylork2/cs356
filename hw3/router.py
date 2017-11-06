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
	printTable(dists)

	return dists

def createMessage(seq_num, router, dists):
	# convert to network byte order 
	seq_ulong = socket.htonl(seq_num)
	rou_ulong = socket.htonl(router)
	dists_flat = dists[router-1]

	#message format (seq_num, router, flattened distance vector)
	message = struct.pack('!ii%ui' %len(dists_flat), seq_ulong, rou_ulong, *dists_flat)
	return message 

def convertMsg(message):
	m = struct.unpack('!ii4i', message)
	seq_num = socket.ntohl(m[0])
	router = socket.ntohl(m[1])
	dists = m[2:]
	return seq_num, router, dists 

def updateDists(dists, sent_dists, sent_router):
	updated = False
	for j in range(0, len(sent_dists)):
		if sent_dists[j] < dists[sent_router-1][j]:
			dists[sent_router-1][j] = sent_dists[j]
			updated = True

	return dists, updated

def bellmanFord(dists, router):
	for j in range(0, len(dists)):
		if j==router-1:
			continue 

		d, fh = D(dists,router-1,j)
		dists[router-1][j] = d

def D(dists, origin, dest):
	min_d = INF
	fh = -1
	for j in range(0, len(dists)):
		d = dists[origin][j] + D(dists, dest, i)[0]
		if d < min_d:
			min_d = d
			fh = j

	return min_d, fh


def printTable(dists):
	print "\t\tDest."
	print "Src.\tR1\tR2\tR3\tR4"
	row = 1
	for l in dists:
		sys.stdout.write("R"+str(row))
		sys.stdout.flush()
		row=row+1
		for d in l:
			sys.stdout.write("\t"+str(d)) 
			sys.stdout.flush()
		print


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
	sock.settimeout(1)

	print >> sys.stderr, 'starting up on %s port %s' %serv_addr

	seq_num = 0

	#Send distance vector to other routers 
	while True and seq_num<10: 

		seq_num = seq_num + 1

		for i in range(1, N+1):
			# Skip itself and router's that are not neighbors 
			if i==R or dists[R-1][i-1] >= INF:
				continue

			message = createMessage(seq_num, R, dists)

			r_addr = (HOST, PORTS[i-1])
			try:
				sent = sock.sendto(message, r_addr)
				print "Sending distance vector to router %s" %i
			except socket.error as msg: 
				print "Caught sendto exception: %s " %msg

		for i in range(1, N+1):
			try:
				data, addr = sock.recvfrom(24)
			except socket.error as msg:
				print "Caught recvfrom exception: %s " %msg	
				continue

		sent_seq_num, sent_router, sent_dists = convertMsg(data)
		print "Received distance vector from router %s" %sent_router
		dists, updated = updateDists(dists, sent_dists, sent_router)

		if updated:
			printTable(dists) 

			
	
	print "Closing socket" 
	sock.close()	


main()