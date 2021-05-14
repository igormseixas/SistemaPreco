/*
	Simple udp client
*/
#include<stdio.h>
#include<winsock2.h>
#include<iostream>
#include<string>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define DEFAULT_BUFLEN 512	//Max length of buffer

using std::cout;
using std::cin;
using std::endl;

// Function to receive data from client and send data to client.
void RecvFromSendTo(SOCKET ClientSocket, sockaddr_in si_other){
    // Variables.
    char buff[DEFAULT_BUFLEN];
    char message[DEFAULT_BUFLEN];
    int slen;
    struct sockaddr_in SenderAddr;

    // Inicialization.
    slen=sizeof(si_other);
    
    // Repeat forever.
    while(1)
	{
		cout << "Enter message : ";
		cin>> message;
		
		// Try to send a message.
		if (sendto(ClientSocket, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		
		//Receive a reply and print it.
		//Clear the buffer by filling null, it might have previously received data.
		memset(buff,'\0', DEFAULT_BUFLEN);
		
        //Try to receive some data, this is a blocking call.
		if (recvfrom(ClientSocket, buff, DEFAULT_BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		
		// Print message from server.
        cout << "Message from server: " << buff << endl;
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	int s, slen=sizeof(si_other);
	char buf[DEFAULT_BUFLEN];
	char message[DEFAULT_BUFLEN];
	WSADATA wsa;

    // Validate the parameters
    if (argc != 3) {
        printf("Usage: %s client <ip/name> <port>\n", argv[0]);
        return 1;
    }

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
	
	//create socket
	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	//setup address structure
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons((u_short)strtoul(argv[2], NULL, 0));
	si_other.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	
    RecvFromSendTo(s,si_other);

	closesocket(s);
	WSACleanup();

	return 0;
}