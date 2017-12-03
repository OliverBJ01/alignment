//
// Created by bernard on 12/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include "camLasAlignment.h"

extern int mcuScannersockfd;

struct clientMssg clientMssg;
struct serverMssg serverMssg;


//=======================================================
// void sendMssgToScanner(uint x, uint y, uint info)
//
// sends TCP message to the mcuScanner server
//
// blocking version; waits until response received before sending
// avoids need for delays in sending
//
// ======================================================
void sendMssgToScanner(uint x, uint y, uint info) {

	int numBytesSent, numBytesRecv;

	//printf("sendMssg() %d %d l:%d \n", x, y, info);

	// reverse; scanner direction is backwards.
	y = SCANNER_DAC_LIMIT - y;
	x = SCANNER_DAC_LIMIT - x;

	clientMssg.scanX = htons(x);
	clientMssg.scanY = htons(y);
	clientMssg.info =  htons(info);

	if((numBytesSent = send(mcuScannersockfd, &clientMssg, sizeof clientMssg, 0)) != sizeof(clientMssg))
		perror("error sending mssg to mcuScanner");

	//blocks waiting for mcuServer response, when not blocking, ie the next lines missing
	// it all just stopped and had to use delays
	numBytesRecv = recv(mcuScannersockfd, &serverMssg, sizeof serverMssg, 0);

	if (numBytesRecv == -1)   // == -1, then nothing received
		perror("No response msuScanner");
}



//=======================================================================
//  int connectToServer(int& mcuScannersockfd)
//
//  creates TCP socket to the mcu scanner server for mcuScannersockfd
//
//========================================================================
int connectToServer() {
	int status;
	struct addrinfo hints, *servinfo;   // address info structure
	char ip4[INET_ADDRSTRLEN];          // space to hold the IPv4 string

	// set hints in structure addrinfo hints
	memset(&hints, 0, sizeof hints);        // clear structure
	hints.ai_family = AF_INET;              // force IPv4
	hints.ai_socktype = SOCK_STREAM;        // TCP stream socket
	//   hints.ai_socktype = SOCK_DGRAM;    // Datagram socket

	//  Note: neither getaddrinfo() or socket() care if the server is connected
	// set up structure addrinfo servinfo
	if ((status = getaddrinfo(SCANNERADDRESS, SCANNERPORT, &hints, &servinfo)) != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

	while (1) {    // if connection times out, keeps trying
		printf("client: connecting to %s, if hung check firewall & server.....\n", SCANNERADDRESS);
		if ((mcuScannersockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
			perror("client: socket error");
			continue;
		}

		// hangs here is server not available, times out with "Connection timed out"
		if (connect(mcuScannersockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
			close(mcuScannersockfd);
			perror("client: connect error");
			continue;
		}
		break;          // breaks on connection
	}

	//fcntl(mcuScannersockfd, F_SETFL, O_NONBLOCK);		// make recv() non-blocking

	struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
	inet_ntop(AF_INET, &(ipv4->sin_addr), ip4, INET_ADDRSTRLEN);

	printf("client: connected to %s\n", ip4);

	// receive and print server response message
	//recv(mcuScannersockfd, &buf, sizeof buf, 0);
	//printf("%s\n", &buf);

	freeaddrinfo(servinfo);             // done with the linked list
	return 0;                           // OK
}
