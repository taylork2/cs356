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

#define TIMEOUT_SEC 1
#define RETRY 10


//error printing
void usage(char *progname, string process, const char *message){
	cerr << "Error: " << progname << " " << process << message << endl;
}

int main(int argc, char* argv[]){

	if (argc != 3){
		usage(argv[0], "command line", " Incorrect number of arguments.");
		return 1;
	}

	char* add = argv[1];
	char* port = argv[2];

	// Initializing socket variables 
	int cli_sock, conn, cli_bind;
	struct addrinfo hints, *cli_info;

	memset(&hints, 0, sizeof hints); //empty struct 

	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP
	hints.ai_flags = 0; //don't need special stuff 

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

	//set socket timeout option to 1 seconds 
	struct timeval t;
	t.tv_sec = TIMEOUT_SEC; 
	t.tv_usec = 0;
	int setop = setsockopt(cli_sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
	if (setop < 0){
		usage(argv[0], "setsockopt ", strerror(errno));
	}


	//Create message 
	char mess[12]; //will store both seqnum & timestamp in here
	int mess_len;
	struct timeval tv;

	char mess_in[12];
	struct addrinfo_storage *serv_info; //info 
	int recv;
	socklen_t rcv_len = sizeof(cli_info);
	char serv_addr[INET6_ADDRSTRLEN];
	
	for (int i=0; i<RETRY; i++){ //retry 10 times 

		//get the sequence num 
		unsigned int seqnum_nbo = htons(i+1);
		memcpy(mess, &seqnum_nbo, 4);
		cout << ntohs(seqnum_nbo) << endl;
		//get the current time
		gettimeofday(&tv, NULL); 
		unsigned long t = 1000000 * tv.tv_sec + tv.tv_usec;
		unsigned long t_nbo = htobe64(t);
		memcpy(mess+4, &t_nbo, 8);
		cout << t << " " << be64toh(t_nbo) << endl;

		// cout << "message" << ntohs(mess[3]) << endl;
		printf("%x %x %x %x\n", mess[0], mess[1], mess[2], mess[3]);


		// send a message to the server
		int send = sendto(cli_sock, mess, sizeof(mess), 0, cli_info->ai_addr, cli_info->ai_addrlen);
		if (send < 0){
			usage(argv[0], "send ", strerror(errno));
		} else if (i == 0 ) {
			cout << "Pinging " << add << ", " << port << ":" << endl;
		}

		//receive a message from server  
		recv = recvfrom(cli_sock, mess_in, mess_len, 0, (struct sockaddr*) &serv_info, &rcv_len);
		
		if (recv >= 0){
			
			//get the host ip that sent the message 
			getpeername(cli_sock, (struct sockaddr*)serv_info, &rcv_len); 
			struct sockaddr_in *s = (struct sockaddr_in *)&serv_info;
    		int port = ntohs(s->sin_port);
		    inet_ntop(AF_INET, &s->sin_addr, serv_addr, sizeof serv_addr);

			cout << "Message received " << serv_addr << endl;
		} else {
			cout << "Ping message " << i+1 << " timed out" << endl;
		}
	}

	return 1;
}