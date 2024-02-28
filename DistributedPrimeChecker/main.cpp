#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET masterSocket;
    struct sockaddr_in server;
    int start, end;

    std::cout << "Enter start point: ";
    std::cin >> start;
    std::cout << "Enter end point: ";
    std::cin >> end;

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    return 0;
}
