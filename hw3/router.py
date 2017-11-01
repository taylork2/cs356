import socket
import sys 

ports = [10001, 10002, 10003, 10004]

# error message 
def usage(progname, msg):
	print(progname, msg)

# Check the commandline arguments 
if sys.argc != 2:
	usage(sys.argv[0], "Incorrect commandline arguments.")
	return 1;

