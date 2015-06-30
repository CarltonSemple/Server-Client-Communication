/* ======================================================>
 * Carlton Semple
 * 
 * The server opens a socket and then reads from a movie text file and 
 * sends results to the client.
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
#include <signal.h> // for child signal handler
#include <sys/wait.h> // wait
#include <errno.h>

void searchMovies(char * userInput, int clientSocket);
int searchString(char * stri, char * sub);
int stringLength(char * stri);

static void childHandler(int sig)
{
	int status;
	pid_t childID;
	
	childID = waitpid(-1, &status, WNOHANG);
	printf("Waiting on Child Process %d\n", (int)childID);
}

int main()
{
   int sock, mlen, addrsize, msgct, chc, chct, clientsock;
   struct sockaddr_in addr;
   char ch, buf[80];
   char *receivedData;
   int receiveCount;
   int childID = -1;	// child id to be obtained by fork
   
   // Prepare to clean up children
   struct sigaction sa;	// signal handler
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = childHandler;
   if (sigaction(SIGCHLD, &sa, NULL) == -1)
   {
	  perror("sigaction");
	  exit(-1);
   }

   // Create a socket
   sock = socket(AF_INET, SOCK_STREAM,0);
   if (sock == -1)
   {   
      perror("opening socket");
      exit(-1);
   }

   
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
		clientsock = accept(sock, (struct sockaddr *) &addr, &addrsize);
		if (clientsock == -1)
		{  
			if(errno == EINTR)	
				continue;		// continue if it's just an interrupted system call
		   perror("on accept");
		   exit(-1);
		}
		printf("connection made with client ");

		// print connection address
		printf ("%s\n", inet_ntoa (addr.sin_addr));

		// Create a new process to handle each client
		//childID = fork();
		switch(childID = fork())
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
		 }
		
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









