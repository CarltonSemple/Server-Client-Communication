/**********************************************************************\
*                Copyright (C) Carlton Semple, 2014.                   *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty.                                                        *
\**********************************************************************/

/* ======================================================>
 * The server opens a socket and then reads UDP messages, responding appropriately 
 * =====================================================================
 */
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>	// string functions like strlen

void searchMovies(char * userInput, int clientSocket);
int searchString(char * stri, char * sub);
int stringLength(char * stri);

struct sockaddr_in server_addr, client_addr;
socklen_t slen=sizeof(client_addr);

int main()
{
   int sockfd, mlen, addrsize, msgct, chc, chct, clientsock;
   
   char ch, buf[80];
   char *receivedData;
   int receiveCount;

   // Create a socket
   // Get socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		printf("socket");
    else
		printf("Socket() successful\n");
	
	// Address setup
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12349);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind to socket
    if (bind(sockfd, (struct sockaddr* ) &server_addr, sizeof(server_addr))==-1)
		printf("bind");
    else
		printf("Bind() successful\n");


	for(;;)
	{
		// Repeatedly receive 
		mlen = recvfrom(sockfd, buf, 80, 0, (struct sockaddr*)&client_addr, &slen);
		if (mlen ==-1)
		{	
            printf("recvfrom()\n");
		}
		printf("length %d\n", mlen);
		
		// make the receiving array large enough to hold the data
		receivedData = (char*)malloc(sizeof(char)*mlen);
		strcpy(receivedData, buf);
		receivedData[mlen-1] = '\0';
		printf("Searching for %s...\n", receivedData);
		
		// Get the result(s) from the file and send them
	    searchMovies(receivedData, sockfd);
		
		/*	   
		// Create a new process to handle each client
		switch(fork())
		{
			case -1:	// error
				printf("Error creating child process\n");
				close(clientsock);
				break;
			
			case 0:	// child process
			
				close(sock);	// close unused copy of listening socket descriptor
				
				// Handle client requests
				while (1)
				{
					// receive the data
					mlen = recv (clientsock, buf, 80, 0);
										
					// make the receiving array large enough to hold the data
					receivedData = (char*)malloc(sizeof(char)*mlen);
					strcpy(receivedData, buf);
					receivedData[mlen-1] = '\0';
					printf("Searching for %s...\n", receivedData);
					
					// Get the result(s) from the file and send them
				   searchMovies(receivedData, clientsock);
				   
				   // Allow the user to quit
				   if(strcmp(receivedData, "quit") == 0 || strcmp(receivedData, "Quit") == 0)
				   	break;
				   
				   free(receivedData);	// clear up the allocated memory
				}
				
				printf("client connection being closed\n");

				// Shutdown the socket to insure that it's handled properly
				shutdown (clientsock, SHUT_RDWR);
				close(clientsock);
				_exit(EXIT_SUCCESS);
			
			default:	// parent
				close(clientsock);	// close unused copy of client socket descriptor
				break;	// loop to accept more connections
		 }*/
		
   }
   
   close(sockfd);
}

/* Get the result(s) from the file and send them to the given socket */
void searchMovies(char * userInput, int clientSocket)
{
	ssize_t fd;
	size_t len = 0;
	ssize_t read;
	char *line = NULL;
	int success = 0;
	int count = 0;		// keep track of which line of the file the program is at

	FILE*fst;
	fst = fopen("movie.txt", "r");
	if (fst == NULL)
	{
		printf("movie file not found\n");
		exit(EXIT_FAILURE);
    }

	// Read the file
	while ((read = getline(&line, &len, fst)) != -1) 
	{		
		if(count == 0) // always send the header to the client
			sendto(clientSocket, line, strlen(line), 0, (struct sockaddr* ) &client_addr, sizeof(client_addr));
		else if(searchString(line, userInput))
		{
			printf("%s", line);
						
			// send matching line to client
			sendto(clientSocket, line, strlen(line), 0, (struct sockaddr* ) &client_addr, sizeof(client_addr));
			success = 1;
		}
		count++;
	}
	
	if(success == 0)
		sendto(clientSocket, "No results found!", 18, 0, (struct sockaddr* ) &client_addr, sizeof(client_addr));
	
	// Send a couple of new lines
	sendto(clientSocket, "\n\n", 3, 0, (struct sockaddr* ) &client_addr, sizeof(client_addr));
	
   fclose(fst);
	return;	
}

int searchString(char * stri, char * sub)
{
	int length = stringLength(stri), sublength = stringLength(sub);
	int i, c=0, successCount = 0;
	
	for(i = 0; i < length && c < sublength; i++)
	{
		if(stri[i] == sub[c])
		{
			successCount++;
			//printf("successcount: %d\n", successCount);
			c++;
		}
		else
		{
			successCount = 0;
			i -= c;
			c = 0;
		}
	}
	
	if(successCount >= sublength)
	{
		printf("index found: %d\n", i-sublength);
		return 1;
	}
	
	return 0;
}

int stringLength(char * stri)
{
	int i = 0;
	
	if(stri == NULL)
		return -1;
	
	while(stri[i] != '\0')
	{
		i++;
	}
	
	return i;
}









