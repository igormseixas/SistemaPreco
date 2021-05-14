#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"

using std::stoi; 
using std::string;
using std::cout;
using std::endl;

// Function to receive data from client and send data to client.
void RecvFromSendTo(SOCKET ServerSocket){
    // Variables.
    char buff[DEFAULT_BUFLEN];
    int iResult, receiveLength, SenderAddrSize;
    struct sockaddr_in SenderAddr;

    // Inicialization.
    SenderAddrSize = sizeof(SenderAddr);

    while(1){
        cout << "Waiting for data..." << endl;
        fflush(stdout);

        // Clear the buffer by filling null.
        memset(buff, '\0', DEFAULT_BUFLEN);

        // Call the recvfrom function to receive datagrams
        // on the bound socket.
        iResult = recvfrom(ServerSocket, buff, DEFAULT_BUFLEN, 0, (SOCKADDR *) & SenderAddr, &SenderAddrSize);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }

        //Print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(SenderAddr.sin_addr), ntohs(SenderAddr.sin_port));
		printf("Data: %s\n" , buff);

        // Send a datagram to the receiver replying with the same data.
        iResult = sendto(ServerSocket, buff, DEFAULT_BUFLEN, 0, (SOCKADDR *) & SenderAddr, SenderAddrSize);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            closesocket(ServerSocket);
            WSACleanup();
            break;
        }
    }
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ServerSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    const char* DEFAULT_PORT = argv[1];

    // Validate the parameters
    if (argc != 2) {
        printf("Usage: %s server <port>\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ServerSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the UDP socket
    iResult = bind( ServerSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    //freeaddrinfo(result);

    // Start loop of communications.
    RecvFromSendTo(ServerSocket);

    // Check if connection is closing in a proper way.
    if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ServerSocket);
            WSACleanup();
            return 1;
    }

    // Shutdown the connection since we're done
    iResult = shutdown(ServerSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    // Cleanup
    closesocket(ServerSocket);
    WSACleanup();

    return 0;
}