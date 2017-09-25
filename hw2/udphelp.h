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
#include <sstream>
#include <math.h>

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


//convert seqnum and time to binary encoding
//store in message to be sent over socket API
//returns the time the message was created as long 
long createMessage(char * message, int seqnum){
	//get the sequence num 
	unsigned int seqnum_nbo = htons(seqnum+1);
	memcpy(message, &seqnum_nbo, 4);

	//get the current time
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	long t = 1000000 * tv.tv_sec + tv.tv_usec;
	long t_nbo = htobe64(t);
	memcpy(message+4, &t_nbo, 8);
	
	return t;
}

int getSeqNum(char mess[]){
	unsigned int seqnum = * (int *) mess;
	return ntohs(seqnum);
}

//will convert timestamp to host order 
double getTimestamp(long timestamp){
	return be64toh(timestamp);			
}

//get the host ip that sent the message 
void getAddr(int socket, struct addrinfo_storage * host_info, socklen_t * len, char * addr[INET_ADDRSTRLEN]){
	
	getpeername(socket, (struct sockaddr*)host_info, len); 
	struct sockaddr_in *s = (struct sockaddr_in *)&host_info;
	// int port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, *addr, sizeof *addr);			
}

double calcOTTinSec(long sent_time, long recv_time){
	return (double)(recv_time - sent_time) * pow(10,-6);

}

double calcRTTinSec(long sent_time){
	//get the current time
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	long t = 1000000 * tv.tv_sec + tv.tv_usec;

	return calcOTTinSec(sent_time, t);
}

