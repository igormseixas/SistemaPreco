#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::stoi;
using std::ofstream;
using std::ifstream;

// Function to receive data from client and send data to client.
void RecvFromSendTo(SOCKET ClientSocket){
    // Variables.
    char buff[DEFAULT_BUFLEN];
    char message[DEFAULT_BUFLEN];
    int iResult, receiveLength, SenderAddrSize;
    struct sockaddr_in SenderAddr;

    // Inicialization.
    SenderAddrSize = sizeof(SenderAddr);

    while(1){
        cout << "Enter message: " << endl;
        cin >> message;

        // Send a message to server.
        iResult = sendto(ClientSocket, message, DEFAULT_BUFLEN, 0, (SOCKADDR *) & SenderAddr, SenderAddrSize);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            break;
        }

        // Call the recvfrom function to receive a replay and print it.

        // Clear the buffer by filling null.
        memset(buff, '\0', DEFAULT_BUFLEN);

        iResult = recvfrom(ClientSocket, buff, DEFAULT_BUFLEN, 0, (SOCKADDR *) & SenderAddr, &SenderAddrSize);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }

        // Print message from server.
        cout << "Message from server: " << buff << endl;
    }
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    const char* DEFAULT_PORT = argv[2];

    // Validate the parameters
    if (argc != 3) {
        printf("Usage: %s client <ip/name> <port>\n", argv[0]);
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

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    
    //freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Start loop of communications.
    RecvFromSendTo(ConnectSocket);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Check the connection is closed in a proper way.
    if ( iResult == 0 )
        printf("Connection closed.\n");
    else
        printf("recv failed with error: %d\n", WSAGetLastError());

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}