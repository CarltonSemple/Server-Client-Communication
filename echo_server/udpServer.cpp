#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string>
//#include <pthread.h> 	// threading

#define BUFLEN 512
#define SRV_IP "192.168.7.2"
using namespace std;

void *sendSensorData(void *socketArg);
void handleCommand(string command);
void sendString(int sockfd, string d);
int readAnalog();

// Argument structure for passing multiple arguments to thread
struct MessageStruct{
	struct sockaddr_in addr;
	int sockfd;
};

struct MessageStruct message;


// Global message struct for passing data

// global socket addresses
struct sockaddr_in my_addr, cli_addr;	
socklen_t slen=sizeof(cli_addr);

int main()
{    
    int sockfd, i; 
    char buf[80];
    char * receivedData;
                
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <  0)
		cout << "socket" << endl;
	
	// Address setup
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(12349);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind to socket
    if (bind(sockfd, (struct sockaddr* ) &my_addr, sizeof(my_addr)) < 0)
		cout << "bind" << endl;
          
    // Thread to send data to C#
    /*pthread_t sendThread;

    // Start send thread
    message.addr = my_addr;
    message.sockfd = sockfd;
    sendThread = pthread_create(&sendThread, NULL, &sendSensorData, NULL);
    */

    while(1)
    {
		int mlen = 0;
		mlen = recvfrom(sockfd, buf, 80, 0, (struct sockaddr*)&cli_addr, &slen);
        if (mlen == -1)
		{	
            cout << "recvfrom()" << endl;
            continue;
		}
		cout << "length: " << mlen << endl;
        // make the receiving array large enough to hold the data
		//receivedData = (char*)malloc(sizeof(char)*mlen);
		//strcpy(receivedData, buf);
		//receivedData[mlen] = '\0';
		buf[mlen] = '\0';		
        //string bufString(receivedData);
        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr* ) &cli_addr, sizeof(cli_addr));
        //send(sockfd, receivedData, strlen(receivedData), 0);
		
		string bufString(buf);
		cout << "received: " << bufString << endl;
		//handleCommand(bufString);
		//free(receivedData);
 	}
    close(sockfd);
	return 0;
}

void *sendSensorData(void *socketArg)
{
	
}

void sendString(int sockfd, string d)
{
	
}

int readAnalog()
{
	
}



