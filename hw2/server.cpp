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

#include "udphelp.h"

using namespace std;

#define PORT "2222"
#define MAXBUF 12 // message is 12 bytes  

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
	hints.ai_flags = AI_PASSIVE; //set the IP address to my own

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

	//initialize variables to receive a message from client  
	char mess_in[MAXBUF];
	char mess_seq[4]; 

	struct addrinfo_storage *cli_info; //info 
	socklen_t rcv_len = sizeof(cli_info); //must initialize this so the size isn't larger than addr size
	char cli_addr[INET6_ADDRSTRLEN];
	int recv;

	//initialize variables to send a message to client 
	char mess_out[MAXBUF];
	int seqNum;
	long recv_t;

	int loss;

	cout << "The server is ready to receive on port: " << PORT << endl;
	
	//infinite loop receiving packets 
	while (true){

		recv = recvfrom(serv_sock, mess_in, MAXBUF, 0, (struct sockaddr*) &cli_info, &rcv_len);

		// generate random number 
		loss = rand() % 11;

		if (recv >= 0){
			
			mess_in[MAXBUF+1]='\0';
			
			memcpy(&mess_seq, &mess_in, 4);
			seqNum = getSeqNum(mess_seq);

			if (loss < 4){ //simulate lost packet 
				cout << "Message with sequence number " << seqNum << " dropped." << endl;
			} else {
				//output trace information
				recv_t = createMessage(mess_out, seqNum);
				cout << "Responding to ping request with sequence number " << seqNum;
				cout << fixed << " received at " <<  recv_t << endl;

				// send a message to the client
				int send = sendto(serv_sock, mess_out, MAXBUF, 0, (struct sockaddr*) &cli_info, rcv_len);
				if (send < 0){
					usage(argv[0], "sendto ", strerror(errno));
					return 1;
				}
			}
		}		
	}

	return 0;
}