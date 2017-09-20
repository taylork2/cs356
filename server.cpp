#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;

void usage(char *progname, const char *message){
	cerr << "Error: " << progname << " " << message << endl;
}
int main(int argc, char* argv[]){

	if (argc != 4){
		usage(argv[0], " Incorrect number of arguments.");
		return 1;
	}

	char* add = argv[1];
	char* port = argv[2];
	int mess_len = atoi(argv[3]);

	cout << mess_len << endl;

	char * message = new char[mess_len];

	int status, serv_sock, conn, serv_bind;
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP

	status = getaddrinfo(add, port, &hints, &servinfo); //fill servinfo 
	if (status != 0){
		usage(argv[0], gai_strerror(status));
	} 

	//get the socket file descriptor
	serv_sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (serv_sock < 0){
		usage(argv[0], gai_strerror(serv_sock));
	}

	//bind to the port 
	serv_bind = bind(serv_sock, servinfo->ai_addr, servinfo->ai_addrlen);
	if (serv_bind != 0){
		usage(argv[0], gai_strerror(serv_bind));
	}

	//connect to the socket 
	conn = connect(serv_sock, servinfo->ai_addr, servinfo->ai_addrlen); 
	if (conn != 0){
		usage(argv[0], gai_strerror(conn));
	}




	return 0;
}