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
#include <limits.h>
#include <algorithm>

#include "udphelp.h"

using namespace std;

#define TIMEOUT_SEC 1
#define RETRY 10

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

	setsock_timeo(argv[0], cli_sock, TIMEOUT_SEC);

	//Create message 
	char mess[12]; //will store both seqnum & timestamp in here

	//Initialize variables for receiving message
	char mess_in[12];
	int mess_len = sizeof(mess); //tell how large of message to receive 
	char mess_time[8];
	struct addrinfo_storage *serv_info; //info 
	int recv;
	socklen_t rcv_len = sizeof(cli_info);
	char serv_addr[INET6_ADDRSTRLEN];

	//packet utility variables 
	long sent_t;
	long recv_t;
	int packets_lost = 0;
	
	double RTT;
	double OTT;

	double maxOTT = 0.0;
	double minOTT = 100000000000.0;
	double totOTT = 0.0;
	double maxRTT = 0.0;
	double minRTT = 100000000000.0;
	double totRTT = 0.0;

	
	for (int i=0; i<RETRY; i++){ //retry 10 times 

		sent_t = createMessage(mess, i); 
		
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
			
			//get the time received by server 
			memcpy(&recv_t, &mess_in[4], 8);
			recv_t = getTimestamp(recv_t);

			OTT = calcOTTinSec(sent_t, recv_t);
			RTT = calcRTTinSec(sent_t);

			//get the min and max for OTT and RTT
			maxOTT = max(OTT, maxOTT);
			minOTT = min(OTT, minOTT);
			maxRTT = max(RTT, maxRTT);
			minRTT = min(RTT, minRTT);

			//total will be used to calculate avg
			totOTT += OTT;
			totRTT += RTT;

			// output to trace 
			cout << "Ping message number " << i+1;
			cout << fixed << " RTT (OTT): " << RTT;
			cout << fixed << " (" << OTT << ") secs" << endl;

		} else {
			packets_lost++;
			cout << "Ping message " << i+1 << " timed out" << endl;
		}

		memset(&mess, 0, sizeof mess); //empty struct 
		memset(&mess_in, 0, sizeof mess_in); //empty struct 
		memset(&recv_t, 0, sizeof recv_t); //empty struct 

	}

	//print out ping stats 
	double packet_loss =  packets_lost / (double)RETRY * 100;
	cout << "\n--- " << argv[1] << " ping statistics ---" << endl;
	cout << RETRY << " packets transmitted, " << RETRY - packets_lost << " received, ";
	cout << packet_loss << "\% packet loss" << endl;

	cout << "rtt min/avg/max = " << minRTT << "/" << totRTT/(double)(RETRY-packets_lost);
	cout << "/" << maxRTT << endl;

	cout << "ott min/avg/max = " << minOTT << "/" << totOTT/(double)(RETRY-packets_lost);
	cout << "/" << maxOTT << endl;


	return 0;
}