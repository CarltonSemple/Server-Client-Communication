/**********************************************************************\
*                Copyright (C) Carlton Semple, 2014.                   *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty.                                                        *
\**********************************************************************/


/* ========================================================> client1.c 
 * Simple client for UDP socket
 * ====================================================================
 */
#define _GNU_SOURCE

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // For exit()
#include <unistd.h> // For close()
#include <string.h>

#include <signal.h> // for killing child thread
#include <pthread.h>

using namespace std;

struct sockaddr_in addr;

// Thread function for continuously reading the server responses
void *readResponse(void * socketArg)
{
	int socketFd, mlen, c;
	char buffer[80];  
	socketFd = (int)socketArg;
	socklen_t slen = sizeof(addr);
	
	while(1)
	{
		mlen = recvfrom(socketFd, buffer, 512, 0, (struct sockaddr*)&addr, &slen);
		//mlen = recv (socketFd, buffer, 80, 0);
		cout << "received: ";
		for (c = 0; c < mlen; c++)
		{
			if (*(buffer+c) == '\0')
				break; //printf ("\n");
			else
				printf ("%c", *(buffer+c));
		}
		cout << endl;
	}
}

int main()
{
	int sockfd, addrsize;
	   
	unsigned int in_address;
   
	// user input variables
	size_t length = 0;
	ssize_t read;
	char *line = NULL;
	
	// Reading thread variable
	pthread_t readThread;

	// Create a socket
   // Get socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		printf("socket");
	
	// Address setup
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12349);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
   
	// Set up the separate thread for reading server responses
	readThread = pthread_create(&readThread, NULL, &readResponse, (void*)sockfd);
   
	// Read user input continuously
	while ((read = getline(&line, &length, stdin)) != -1) 
	{
		// send line to server
		//
		line[strlen(line)-1] = '\0';
		sendto(sockfd, line, strlen(line), 0, (struct sockaddr* ) &addr, sizeof(addr));
		//send(sockfd, line, strlen(line), 0);
				
		// Allow the user to quit
		if(strcmp(line, "quit") == 0 || strcmp(line, "Quit") == 0)
			break;	
				
		printf("\n");
		free(line);
		line = NULL;
	}
	
	// End the reading thread
	pthread_join(readThread, NULL);

   printf ("Client is done\n");
   close(sockfd);
}
