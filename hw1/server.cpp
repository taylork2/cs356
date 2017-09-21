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

using namespace std;

#define TIMEOUT_SEC 1
#define RETRY 3

void usage(char *progname, string process, const char *message){
	cerr << "Error: " << progname << " " << process << message << endl;
}
int main(int argc, char* argv[]){

	if (argc != 4){
		usage(argv[0], "command line", " Incorrect number of arguments.");
		return 1;
	}

	char* add = argv[1];
	char* port = argv[2];
	int mess_len = atoi(argv[3]);

	char * message = new char[mess_len];

	int status, serv_sock, conn, serv_bind;
	struct addrinfo hints, *serv_info;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP
	// hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(add, port, &hints, &serv_info); //fill serv_info 
	if (status != 0){
		usage(argv[0], "getaddrinfo ", gai_strerror(errno));
		return 1;
	} 

	//get the socket file descriptor
	serv_sock = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol);
	if (serv_sock < 0){
		usage(argv[0], "socket ", strerror(errno));
		return 1;
	}

	//set socket timeout option to 1 seconds 
	struct timeval t;
	t.tv_sec = TIMEOUT_SEC; 
	t.tv_usec = 0;
	int setop = setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&t, sizeof(t));
	if (setop < 0){
		usage(argv[0], "setsockopt ", strerror(errno));
		return 1;
	}

	//bind to the port 
	serv_bind = bind(serv_sock, serv_info->ai_addr, serv_info->ai_addrlen);
	if (serv_bind != 0){
		usage(argv[0], "bind ", strerror(errno));
		return 1;
	}

	//connect to the socket (ONLY WITH TCP)
	// conn = connect(serv_sock ,serv_info->ai_addr, serv_info->ai_addrlen); 
	// if (conn != 0){
	// 	usage(argv[0], "connect ", gai_strerror(conn));
	// }

	//receive a message from server  
	char * mess_in;
	struct addrinfo *XXXXXXXX; //info 
	int recv;
	socklen_t rcv_len;
	recv = recvfrom(serv_sock, mess_in, mess_len, 0, (struct sockaddr*) &XXXXXXXX, &rcv_len);
	if (!(recv < 0)){
		usage(argv[0], "recvfrom ", strerror(errno));
		return 1;
	} else {
		//send a message to the server
		struct sockaddr_storage *dest; 
		int send = sendto(serv_sock, message, mess_len, 0, serv_info->ai_addr, serv_info->ai_addrlen);
		if (send != 0){
			usage(argv[0], "sendto ", strerror(errno));
			return 1;
		}
	}


	return 0;
}