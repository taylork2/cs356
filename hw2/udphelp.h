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
void createMessage(char * message, int seqnum){
	//get the sequence num 
	unsigned int seqnum_nbo = htons(seqnum+1);
	memcpy(message, &seqnum_nbo, 4);
	// cout << ntohs(seqnum_nbo) << endl;

	//get the current time
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	unsigned long t = 1000000 * tv.tv_sec + tv.tv_usec;
	unsigned long t_nbo = htobe64(t);
	memcpy(message+4, &t_nbo, 8);
	printf("%lu\n", t);
	// cout << t << " " << be64toh(t_nbo) << endl;

	// cout << "message" << ntohs(mess[3]) << endl;
	// printf("%x %x %x %x\n", message[0], message[1], message[2], message[3]);
	printf("%x %x %x %x %x %x %x %x\n", message[4], message[5], message[6], message[7], message[8], message[9], message[10], message[11]);
}

//converts hex to long integer 
long hexToLong(char message[]){
	unsigned long x;
	stringstream ss;
	ss << hex << message;
	ss >> x;
	return x;
}

int hexToInt(char message[], int len){
	unsigned int x;
	x = (message[0] << 24) | (message[1] << 16) | (message[2] << 8) | message[3];
	return ntohs(x);
}

int hexToInt2(char a[], int num){
	a[3] = (num>>24) & 0xFF;
	a[2] = (num>>16) & 0xFF;
	a[1] = (num>>8) & 0xFF;
	a[0] = num & 0xFF;
	return num;
}

long getSeqNum(char message[]){
	char seq[4];
	for (int i=0; i<4; i++){
		seq[i] = message[i];
	}
	return hexToLong(seq);
}

long getTimestamp(char message[]){
	char timestamp[8];
	int j = 0;
	for (int i=4; i<12; i++){
		timestamp[j] = message[i];
		j++;
	}
	return hexToLong(timestamp);
}

//get the host ip that sent the message 
void getAddr(int socket, struct addrinfo_storage * host_info, socklen_t * len, char * addr[INET_ADDRSTRLEN]){
	
	getpeername(socket, (struct sockaddr*)host_info, len); 
	struct sockaddr_in *s = (struct sockaddr_in *)&host_info;
	// int port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, *addr, sizeof *addr);			
}

