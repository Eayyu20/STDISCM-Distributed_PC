#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock init failed.\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // use localhost IP

    connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));

    int range[2] = { 1, 100 }; // example range
    send(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    int result;
    recv(clientSocket, reinterpret_cast<char*>(&result), sizeof(result), 0);
    std::cout << "Result received: " << result << std::endl;

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
