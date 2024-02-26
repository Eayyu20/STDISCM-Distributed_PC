#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Could not create socket : " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Setup address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    int range[2] = { 1, 100 }; // example range
    send(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    int result;
    recv(clientSocket, reinterpret_cast<char*>(&result), sizeof(result), 0);
    std::cout << "Result received: " << result << std::endl;

    // Close socket
    closesocket(clientSocket);
    WSACleanup();
    std::cout << "Connection closed." << std::endl;

    return 0;
}