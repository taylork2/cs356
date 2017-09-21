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

	status = getaddrinfo(NULL, port, &hints, &serv_info); //fill serv_info 
	if (status != 0){
		usage(argv[0], "getaddrinfo ", gai_strerror(status));
	} 

	//get the socket file descriptor
	serv_sock = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol);
	if (serv_sock < 0){
		usage(argv[0], "socket ", strerror(errno));
	}

	//bind to the port 
	serv_bind = bind(serv_sock, serv_info->ai_addr, serv_info->ai_addrlen);
	if (serv_bind != 0){
		usage(argv[0], "bind ", strerror(errno));
	}

	//set socket timeout option to 1 seconds 
	struct timeval t;
	t.tv_sec = TIMEOUT_SEC; 
	t.tv_usec = 0;
	int setop = setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
	if (setop < 0){
		usage(argv[0], "setsockopt ", strerror(errno));
	}

	//connect to the socket (ONLY WITH TCP)
	// conn = connect(serv_sock ,serv_info->ai_addr, serv_info->ai_addrlen); 
	// if (conn != 0){
	// 	usage(argv[0], "connect ", gai_strerror(conn));
	// }

	//send a message to the server
	struct sockaddr_storage *dest; 
	int send = sendto(serv_sock, message, mess_len, 0, (struct sockaddr*) &serv_sock, sizeof(serv_sock));
	if (send != 0){
		usage(argv[0], "send ", strerror(errno));
	} else {
		cout << "Pinging " << add << " " << port << " " << message << endl;
	}

	//receive a message from server  
	char * mess_in;
	struct addrinfo *cli_info; //info 
	int recv;
	socklen_t rcv_len;
	for (int i=0; i<=RETRY; i++){ //retry 3 times 
		recv = recvfrom(serv_sock, mess_in, mess_len, 0, (struct sockaddr*) &cli_info, &rcv_len);
		if (!(recv < 0)){
			cout << "Message received " << mess_in << endl;
			break;
		}
	}

	return 0;
}