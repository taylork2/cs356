// HW1 Taylor Tu
// cs356 Thompson 

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

using namespace std;

//error printing
void usage(char *progname, string process, const char *message){
	cerr << "Error: " << progname << " " << process << message << endl;
}

void setsock_timeo(char *progname, int socket, int seconds){
	//set socket timeout option to 1 seconds 
	struct timeval t;
	t.tv_sec = seconds; 
	t.tv_usec = 0;
	int setop = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
	if (setop < 0){
		usage(progname, "setsockopt ", strerror(errno));
	}
}

