#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void processClient(SOCKET clientSocket) {
    // example test
    int range[2]; // start and end range
    recv(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    int start = range[0];
    int end = range[1];
    int sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }

    send(clientSocket, reinterpret_cast<char*>(&sum), sizeof(sum), 0);
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock init failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Master server waiting for connections...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        processClient(clientSocket);
    }

    WSACleanup();
    return 0;
}
