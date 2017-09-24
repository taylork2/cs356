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

using namespace std;

#define PORT "2222"
#define MAXBUF 12 // 8k max size 

void usage(char *progname, string process, const char *message){
	cerr << "Error: " << progname << " " << process << message << endl;
}
int main(int argc, char* argv[]){

	if (argc != 1){
		usage(argv[0], "command line", " Incorrect number of arguments.");
		return 1;
	}

	int status, serv_sock, conn, serv_bind;
	struct addrinfo hints, *serv_info;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP
	hints.ai_flags = 0; //set the IP address to my own

	status = getaddrinfo(NULL, PORT, &hints, &serv_info); //fill serv_info 
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

	//bind to the port 
	serv_bind = bind(serv_sock, serv_info->ai_addr, serv_info->ai_addrlen);
	if (serv_bind != 0){
		usage(argv[0], "bind ", strerror(errno));
		return 1;
	}

	//receive a message from client  
	char mess_in[MAXBUF];
	struct addrinfo_storage *cli_info; //info 
	int recv;
	socklen_t rcv_len = sizeof(cli_info); //must initialize this so the size isn't larger than addr size
	char cli_addr[INET_ADDRSTRLEN];

	cout << "The server is ready to receive on port: " << PORT << endl;
	
	//infinite loop receiving packets 
	while (true){
		recv = recvfrom(serv_sock, mess_in, MAXBUF-1, 0, (struct sockaddr*) &cli_info, &rcv_len);

		if (recv >= 0){

			getpeername(serv_sock, (struct sockaddr*)cli_info, &rcv_len); 
			struct sockaddr_in *s = (struct sockaddr_in *)&cli_info;
			int port = ntohs(s->sin_port);
		    inet_ntop(AF_INET, &s->sin_addr, cli_addr, sizeof cli_addr);
				
			mess_in[MAXBUF]='\0';
			cout << cli_addr << " sent message: " << mess_in << endl;

			// send a message to the client
			int send = sendto(serv_sock, mess_in, MAXBUF, 0, (struct sockaddr*) &cli_info, rcv_len);
			if (send < 0){
				usage(argv[0], "sendto ", strerror(errno));
				return 1;
			}
		}
		
	}


	return 0;
}