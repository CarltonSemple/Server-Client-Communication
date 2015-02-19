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
 * Simple client for Internet communication using stream sockets.
 * This program simply accesses a server socket and writes a few messages.
 * Then it closes the socket and terminates.
 * ====================================================================
 */
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // For exit()
#include <unistd.h> // For close()
#include <string.h>

#include <signal.h> // for killing child thread
#include <pthread.h>

// Thread function for continuously reading the server responses
void *readResponse(void * socketArg)
{
	int socketFd, mlen, c;
	char buffer[80];  
	socketFd = (int)socketArg;
	
	while(1)
	{
		mlen = recv (socketFd, buffer, 80, 0);
		for (c = 0; c < mlen; c++)
		{
			if (*(buffer+c) == '\0')
				printf ("\n");
			else
				printf ("%c", *(buffer+c));
		}
	}
}

int main()
{
   int sock, addrsize;
    
   struct sockaddr_in addr;
   unsigned int in_address;
   
   // user input variables
   size_t length = 0;
   ssize_t read;
	char *line = NULL;
	
	// Reading thread variable
	pthread_t readThread;

   // Open a socket for Internet stream services.
   sock = socket(AF_INET, SOCK_STREAM,0);
   if (sock == -1)
   {   perror("opening socket");
       exit(-1);
   }

   // Address Setup ********
   addr.sin_family = AF_INET;						// address family
   addr.sin_port = htons (12345);					// Port for the server
   in_address = 127 << 24 | 0 << 16 | 0 << 8 | 1;	// Local Address
   addr.sin_addr.s_addr = htonl (in_address);
   if (connect (sock, (struct sockaddr *) &addr, sizeof (struct sockaddr_in)) == -1)
   {   
      perror("on connect");
      exit(-1);
   } 
   
   // Set up the separate thread for reading server responses
   readThread = pthread_create(&readThread, NULL, &readResponse, (void*)sock);
   
	// Read user input continuously
	while ((read = getline(&line, &length, stdin)) != -1) 
	{
		send (sock, line, strlen(line), 0);
				
		// Allow the user to quit
		line[strlen(line)-1] = '\0';
		if(strcmp(line, "quit") == 0 || strcmp(line, "Quit") == 0)
			break;	
				
		printf("\n");
	}
	
	// End the reading thread
	pthread_join(readThread, NULL);

   // Communicate that it's going to shut down
   if (shutdown(sock, 1) == -1)
   {  
      perror("on shutdown");
      exit(-1);
   }
   printf ("Client is done\n");
   close(sock);
}
