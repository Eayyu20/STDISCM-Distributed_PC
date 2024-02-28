#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("103.231.240.86"); // Change to the server's IP address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // Receive data from the server
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::cout << "Received message: " << buffer << std::endl;
    }

    // Close socket
    closesocket(clientSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
