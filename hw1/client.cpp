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

	char message[mess_len];
	memset(&message, 0 , sizeof(message));

	int cli_sock, conn, cli_bind;
	struct addrinfo hints, *cli_info;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP

	//store address information in cli_info
	int status = getaddrinfo(add, port, &hints, &cli_info); //fill cli_info 
	if (status != 0){
		usage(argv[0], "getaddrinfo ", gai_strerror(status));
	} 

	//get the socket file descriptor
	cli_sock = socket(cli_info->ai_family, cli_info->ai_socktype, cli_info->ai_protocol);
	if (cli_sock < 0){
		usage(argv[0], "socket ", strerror(errno));
	}

	//bind to the port 
	cli_bind = bind(cli_sock, cli_info->ai_addr, cli_info->ai_addrlen);
	if (cli_bind != 0){
		usage(argv[0], "bind ", strerror(errno));
	}

	//set socket timeout option to 1 seconds 
	struct timeval t;
	t.tv_sec = TIMEOUT_SEC; 
	t.tv_usec = 0;
	int setop = setsockopt(cli_sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
	if (setop < 0){
		usage(argv[0], "setsockopt ", strerror(errno));
	}

	//send a message to the server
	struct sockaddr_storage *dest; 
	int send = sendto(cli_sock, message, sizeof(message), 0, cli_info->ai_addr, cli_info->ai_addrlen);
	if (send < 0){
		usage(argv[0], "send ", strerror(errno));
	} else {
		cout << "Pinging " << add << " " << port << " " << message << endl;
	}

	//receive a message from server  
	char * mess_in;
	struct addrinfo *serv_info; //info 
	int recv;
	socklen_t rcv_len = sizeof(cli_info);
	for (int i=0; i<=RETRY; i++){ //retry 3 times 
		recv = recvfrom(cli_sock, mess_in, mess_len, 0, (struct sockaddr*) &serv_info, &rcv_len);
		if (!(recv < 0)){
			cout << "Message received " << serv_info->addr << " " << mess_in << endl;
			return 0;
		}
	}

	usage(argv[0], "receive ", "timeout");

	return 1;
}