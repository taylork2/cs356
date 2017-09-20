#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;

void usage(char *progname, string message){
	cerr << "Error: " << progname << message << endl;
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

	// struct sockaddr_in serv_addr, cli_addr;

	// serv_addr.sin_family = AF_INET;
	// serv_addr.sin_port = htons(port);

	// int client_sock = socket(AF_INET, SOCK_DGRAM, 0);

	// int error = bind(client_sock, (struct sockaddr_in *) serv_addr, sizeof(serv_addr));

	// if (error < 0){
	// 	usage(argv[0], "Problem binding.");
	// }

	int status, serv_sock, conn;
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP

	status = getaddrinfo(add, port, &hints, &servinfo); //fill servinfo 
	if (status != 0){
		usage(argv[0], " getaddrinfo issue.");
	} 

	//create the server socket 
	serv_sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (serv_sock < 0){
		usage(argv[0], " issue creating socket.");
	}

	//connect to the socket 
	conn = connect(serv_sock, servinfo->ai_addr, servinfo->ai_addrlen); 
	if (conn != 0){
		usage(argv[0], " connecting issue.");
	}



	return 0;
}