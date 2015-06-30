/* ======================================================>
 * Carlton Semple
 * 
 * The server opens a socket and then reads from a movie text file and 
 * sends results to the client.
 * =====================================================================
 */
#define _GNU_SOURCE
#define CLIENT_ARRAY_SIZE 50

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>	// string functions like strlen
#include <sys/wait.h> // wait
#include <errno.h>
// includes for select
#include <sys/time.h>
#include <sys/types.h>

void searchMovies(char * userInput, int clientSocket);
int searchString(char * stri, char * sub);
int stringLength(char * stri);

int main()
{
   int sock, mlen, addrsize, msgct, chc, chct, clientsock;
   struct sockaddr_in addr;
   char ch, buf[80];
   char *receivedData;
   int receiveCount;
   
   int retval = 0;
   int highestFdPlusOne;
   int fdCount = 0;
   int clientSockets[CLIENT_ARRAY_SIZE] = {-1}; // array of client socket descriptors
   fd_set rfds;	// file descriptor set for select
   FD_ZERO(&rfds);	// clear the set

   // Create a socket
   sock = socket(AF_INET, SOCK_STREAM,0);
   if (sock == -1)
   {   
      perror("opening socket");
      exit(-1);
   }

	// Add socket fd to set
	clientSockets[fdCount] = sock;
	fdCount++;
	FD_SET(sock, &rfds);
	highestFdPlusOne = sock+1;
   
    /* Bind a name to the socket.  Since the server will bind with
    * any client, the machine address is zero or INADDR_ANY.  The port
    * has to be the same as the client uses.*/
   addr.sin_family = AF_INET;
   addr.sin_port = htons (12345);
   addr.sin_addr.s_addr = htonl (INADDR_ANY);

   if (bind(sock, (struct sockaddr *) &addr, sizeof (struct sockaddr_in)) == -1) 
   {  
      perror ("error: bind");
      exit (-1);
   }

   // Make the socket available for potential clients.
   if (listen(sock,1) == -1)  
   {  
      perror("on listen");
      exit(-1);
   }

	for(;;)
	{
		// Wait for a client to connect.
		addrsize = sizeof(struct sockaddr_in);
		
		// Clear set
		FD_ZERO(&rfds);
		// Add sockets to set
		for(int i = 0; i < fdCount; i++)
			FD_SET(clientSockets[i], &rfds);
		
		// Debuggg
		printf("before select\n");
		
		retval = select(highestFdPlusOne, &rfds, NULL, NULL, NULL);
		
		// Debuggg
		printf("afer select\n");
		
		if(retval > 0) // number of file descriptors in set that are ready
		{
			// server is ready
			if(FD_ISSET(sock, &rfds))
			{			
				// Get new client
				clientsock = accept(sock, (struct sockaddr *) &addr, &addrsize);
				if(clientsock >= highestFdPlusOne)
					highestFdPlusOne = clientsock + 1;
			
				if (clientsock == -1)
				{  
					if(errno == EINTR)	
						continue;		// continue if it's just an interrupted system call
				   perror("on accept");
				   exit(-1);
				}	
				// Store the client's socket descriptor
				clientSockets[fdCount] = clientsock;
				fdCount++;

				// print connection address
				printf ("%s\n", inet_ntoa (addr.sin_addr));
				
			}
		}
		
		// Handle client requests for all clients, not the server
		for(int i = 1; i < fdCount; i++)
		{
			if(!FD_ISSET(clientSockets[i], &rfds))
				continue;
				
			// receive the data
			mlen = recv (clientSockets[i], buf, 80, 0);
			if(mlen == 0)
			{
				// remove from set
				FD_CLR(clientSockets[i], &rfds);
				printf("%d received\n", mlen);
				
				// Shutdown the socket to insure that it's handled properly
				shutdown (clientSockets[i], SHUT_RDWR);
				close(clientSockets[i]);
			   
				// Remove fd from fd array
				for(int c = 0; c < fdCount; c++)
				{
					if(clientSockets[c] == clientSockets[i])
					{
						// remove by shifting
						int h = c;
						for(int j = c+1; j < fdCount; j++)
							clientSockets[h] = clientSockets[j];
						fdCount--;
						break;
					}
				}	
			}
								
			// make the receiving array large enough to hold the data
			receivedData = (char*)malloc(sizeof(char)*mlen);
			strcpy(receivedData, buf);
			receivedData[mlen-1] = '\0';
			
			// Allow the user to quit
			if(strcmp(receivedData, "quit") == 0 || strcmp(receivedData, "Quit") == 0 || strcmp(receivedData, "") == 0)
			{
			   free(receivedData);	// clear up the allocated memory

			   // remove from set
			   FD_CLR(clientSockets[i], &rfds);
			   printf("client connection being closed\n");

			   // Shutdown the socket to insure that it's handled properly
			   shutdown (clientSockets[i], SHUT_RDWR);
			   close(clientSockets[i]);
			   
			   // Remove fd from fd array
			   for(int c = 0; c < fdCount; c++)
			   {
				   if(clientSockets[c] == clientSockets[i])
				   {
					   // remove by shifting
					   int h = c;
					   for(int j = c+1; j < fdCount; j++)
							clientSockets[h] = clientSockets[j];
						fdCount--;
						break;
				   }
			   }			   
			   
			   continue;
			}			
			
			// Get the result(s) from the file and send them
			printf("Searching for %s...\n", receivedData);
			searchMovies(receivedData, clientSockets[i]);
					   
			free(receivedData);	// clear up the allocated memory
		}
				
		//_exit(EXIT_SUCCESS);
   }
   
   //close(sock);
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
	//fst = fopen("/home/network/Documents/cis486/Project1/movie.txt", "r");
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
				send(clientSocket, line, strlen(line), 0);	
		else if(searchString(line, userInput))
		{
			printf("%s", line);
						
			// send matching line to client
			send(clientSocket, line, strlen(line), 0);
			success = 1;
		}
		count++;
	}
	
	if(success == 0)
		send(clientSocket, "No results found!", 18, 0);
	
	// Send a couple of new lines
	send(clientSocket, "\n\n", 3, 0);
	
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









