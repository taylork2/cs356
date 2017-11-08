import socket
import sys 
import ctypes
import struct 


INF = 10000; #infinity 

N = 4 #number of routers 
HOST = 'localhost'
PORTS = [10001, 10002, 10003, 10004]

# Distance vector 
DISTANCES = [[0,1,4,8], 
	 		 [1,0,2,INF],
	 		 [4,2,0,1],
	 		 [8,INF,1,0]]
NEIGHBOR = [ [False, True, True, True ],
			 [True, False, True, False],
			 [True, True, False, True ],
			 [True, False, True, False] ]



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

# Bellman Ford ALgorithm 
def bellmanFord(dists, router, firsthop_table):
	updated = False

	#Relax the distances for all the vertices 
	for v in range(0, len(dists)):
		for i in range(0, len(dists)):
			for j in range(0, len(dists)):
				if not NEIGHBOR[i][j]:
					continue 

				if dists[router-1][j] > dists[router-1][i] + dists[i][j]:
					dists[router-1][j] = dists[router-1][i] + dists[i][j]
					firsthop_table[j] = firsthop_table[i]
					print "Bellman Ford Update"
					printTable(dists) 
					updated = True

	return updated, dists, firsthop_table

def printTable(dists):
	print "==============================================="
	print "\t|\t  Dest."
	print " Src.\t| R1\t| R2\t| R3\t| R4"
	print "--------|-------|-------|-------|-------"
	row = 1
	for l in dists:
		sys.stdout.write("R"+str(row))
		sys.stdout.flush()
		row=row+1
		for d in l:
			sys.stdout.write("\t| "+str(d)) 
			sys.stdout.flush()
		print

	print "==============================================="


def printFT(router, ft):
	print "==============================================="
	print "Router %s Forwarding Table:" %router
	print "Dest.\t| Next Hop"
	print "--------|---------"

	r = 1
	for i in ft:
		sys.stdout.write("R"+str(r))
		sys.stdout.flush()
		r = r+1
		sys.stdout.write("\t|"+str(i)) 
		sys.stdout.flush()
		print 

	print "==============================================="



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

	FH = [ 1, 2, 3, 4 ] #first hop table 
	FH[R-1] = "local"
	
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

	# set socket timeout of 1 second  
	sock.settimeout(20)

	print >> sys.stderr, 'starting up on %s port %s' %serv_addr

	seq_num = 0

	#Send distance vector to other routers 
	while True and seq_num<100: 

		seq_num = seq_num + 1

		for i in range(1, N+1):
			# Skip router's that are not neighbors (including itself)
			if not NEIGHBOR[R-1][i-1]:
				continue

			message = createMessage(seq_num, R, dists)

			r_addr = (HOST, PORTS[i-1])
			try:
				sent = sock.sendto(message, r_addr)
				print "Sending distance vector to router {} > {}".format(i, dists[R-1])
			except socket.error as msg: 
				print "Caught sendto exception: %s " %msg


		breakLoop = False
		while True:
			try:
				data, addr = sock.recvfrom(24) #24 bytes 
			except socket.error as msg:
				print "Caught recvfrom exception: %s " %msg	
				breakLoop = True
				ft_updated, dists, FH = bellmanFord(dists, R, FH)
				if ft_updated: 
					breakLoop = False
					printFT(R, FH)


			if breakLoop == True:
				break

			sent_seq_num, sent_router, sent_dists = convertMsg(data)
			print "Received distance vector from router {} > {}".format(sent_router, sent_dists)
			dists, updated = updateDists(dists, sent_dists, sent_router)

			if updated:
				printTable(dists) 
				break

			ft_updated, dists, FH = bellmanFord(dists, R, FH)
			if ft_updated: 
				printFT(R, FH)
				break

		# If the socket does not recv message, socket will time out and break
		if breakLoop == True:
			break		
	
	print "Closing socket" 
	sock.close()

	printFT(R, FH)	


main()