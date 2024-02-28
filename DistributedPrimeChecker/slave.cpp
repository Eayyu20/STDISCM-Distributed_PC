#include <iostream>
#include <winsock2.h>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

void processTask(SOCKET sock);

int main() {
    WSADATA wsa;
    SOCKET slaveSocket, newSocket;
    struct sockaddr_in server, client;
    int c = sizeof(struct sockaddr_in);

    WSAStartup(MAKEWORD(2, 2), &wsa);
    slaveSocket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080); // Different port for slave server

    bind(slaveSocket, (struct sockaddr*)&server, sizeof(server));
    listen(slaveSocket, 3);

    std::cout << "Slave Server waiting for tasks..." << std::endl;
    while ((newSocket = accept(slaveSocket, (struct sockaddr*)&client, &c)) != INVALID_SOCKET) {
        processTask(newSocket);
    }

    closesocket(slaveSocket);
    WSACleanup();
    return 0;
}

void processTask(SOCKET sock) {
    int range[2]; // To store start and end points
    recv(sock, (char*)&range, sizeof(range), 0); // Receive task

    // Compute sum of numbers in range
    int start = range[0], end = range[1], sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }

    // Send result back
    send(sock, (char*)&sum, sizeof(sum), 0);
    std::cout << "Task completed, result sent back." << std::endl;
}
