/*
	Simple UDP Server
*/

#include<stdio.h>
#include <bits/stdc++.h>
#include<winsock2.h>
#include<iostream>
#include <algorithm> 
#include<fstream>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define DEFAULT_BUFLEN 512	//Max length of buffer

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::stold;

// Function to convert degrees to radians.
long double toRadians(const long double degree){
    long double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}

// Function to calculate de geography distance using the Harvasine formula.
long double geoDistance(long double lat1, long double long1,
                        long double lat2, long double long2){
    //Variables.
    long double dlong, dlat, earth_radius, distance;

    // Convert latitudes.
    lat1 = toRadians(lat1); long1 = toRadians(long1);
    lat2 = toRadians(lat2); long2 = toRadians(long2);

    // Apply haversine formula.
    dlat = lat2 - lat1;
    dlong = long2 - long1;

    distance = pow(sin(dlat / 2), 2) +
                    cos(lat1) * cos(lat2) *
                    pow(sin(dlong / 2), 2);

    distance = 2 * asin(sqrt(distance));

    // Radius of the earth.
    // Kilometers = 6371.
    // Miles = 3956.
    earth_radius = 6371;

    //Multiply by earth radius
    distance = distance * earth_radius;

    return distance;
}

// Function to split a string given a delimiter.
void tokenize(string *token, string s, string del)
{
    int start = 0;
    int end = s.find(del);
    int position = 0;

    while (end != -1) {
        token[position] = s.substr(start, end - start);
        start = end + del.size();
        end = s.find(del, start);
        position++;
    }
    token[position] = s.substr(start, end - start);
}

// Function to save data to file.
// Function to place ships.
bool saveDataToFile(char *buff){ 
    // Variables.
    ofstream file("data.txt", ios::app);
    bool out = false;

    // Write file.
    if(file.is_open()){
        file << buff << endl;
        out = true;
    } else{
        cout << "Error open file!";
        out = false;
    }
    
    // Close file.
    file.close();

    return out;
}

// Function to find the cheapest fuel price given a certain radius.
int findCheapest(int fuel_type, long double radius, long double latitude, long double longitude){
    //Variables.
    ifstream file("data.txt");
    string line, token[5];
    int fuel_price = INT_MAX, line_numbers;
    long double file_latitude, file_longitude;

    // Read file.
    if(file.is_open()){
        // Count number of lines of a file.
        line_numbers = count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
        file.seekg(0, ios::beg); // Reset the file seek pointer.

        //Start the file reading.
        while(getline(file, line)){
            // Threat data.
            // Get only the fuel type that we need.
            if(stoi(string(1, line[2])) == fuel_type){
                // Split line.
                tokenize(token, line,",");
                // Get only the data in the certain radius.
                file_latitude = stold(token[3]);
                file_longitude = stold(token[4]);
                cout << "Distance beetween coordinates for test: " << geoDistance(latitude, longitude, file_latitude, file_longitude) << endl;
                if(geoDistance(latitude, longitude, file_latitude, file_longitude) < radius){
                    //Check if the fuel price is smaller than the stored vars.
                    if(stoi(token[2]) < fuel_price){
                        fuel_price = stoi(token[2]);
                    }
                }      
            }
        }
    } else{
        cout << "Error open file!";
    }

    return fuel_price;
}

// Function to receive data from client and send data to client.
void RecvFromSendTo(SOCKET ClientSocket){
    // Variables.
    char buff[DEFAULT_BUFLEN];
    char message[DEFAULT_BUFLEN];
    int slen, recv_len, fuel_type=0, cheapest_price; 
    long double radius = 0, latitude, longitude;
    struct sockaddr_in si_other;
    string msg_to_client[5];

    // Inicialization.
    slen=sizeof(si_other);
    
    // Keep listen data.
    while(1)
	{
		cout << "Waiting for data..." << endl;
		fflush(stdout);
		
        //Clear the buffer by filling null, it might have previously received data.
		memset(buff,'\0', DEFAULT_BUFLEN);

        //Try to receive some data, this is a blocking call.
		if ((recv_len = recvfrom(ClientSocket, buff, DEFAULT_BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		}

        // Check if it is data of find type.
        if(buff[0] == 'D'){
            cout << "Server received a DATA command from: " << inet_ntoa(si_other.sin_addr) << ":" << ntohs(si_other.sin_port) << endl;
            
            if(saveDataToFile(buff)){
                cout << "Data: " << buff << " was succesfully saved to file." << endl;
                memset(buff,'\0', DEFAULT_BUFLEN);
                strncpy(buff,"Data: was succesfully saved to file.", 36); 
            }else{
                cout << "Fail to save Data to file." << endl;
                memset(buff,'\0', DEFAULT_BUFLEN);
                strncpy(buff,"Fail to save Data to file.", 36); 
            }            
        }else{
            // Certify that is a find.
            if(buff[0] == 'P'){
                // Create a tmp string to deal with buff.
                string tmp(buff);
                // Tokenzine the string tmp using "," as token.
                tokenize(msg_to_client, tmp, ",");
                // Transform data to its native.
                fuel_type = stoi(msg_to_client[1]);
                radius = stold(msg_to_client[2]);
                latitude = stold(msg_to_client[3]);
                longitude = stold(msg_to_client[4]);
                // Find the cheapest fuel.
                cheapest_price = findCheapest(fuel_type, radius, latitude, longitude);
                cout << "Cheapest price found: " << cheapest_price << endl;
                // Clear buffer and send to client.
                memset(buff,'\0', DEFAULT_BUFLEN);
                strncpy(buff,std::to_string(cheapest_price).c_str(), 4); 
            }
        }

		// Reply the client with the same data.
		if (sendto(ClientSocket, buff, DEFAULT_BUFLEN , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv)
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[DEFAULT_BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other) ;
	
    // Validate the parameters
    if (argc != 2) {
        printf("Usage: %s server <port>\n", argv[0]);
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
	
	//Create a socket
	if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
	printf("Socket created.\n");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons((u_short)strtoul(argv[1], NULL, 0));
	
	//Bind
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

    RecvFromSendTo(s);

	closesocket(s);
	WSACleanup();
	
	return 0;
}