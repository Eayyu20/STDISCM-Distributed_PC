#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <cstring>

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

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1; // Return with error code
    }

    masterSocket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
    server.sin_port = htons(8080); // Connecting to slave server port

    if (connect(masterSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connect failed." << std::endl;
        closesocket(masterSocket);
        WSACleanup();
        return 1;
    }

    int range[2] = { start, end };
    send(masterSocket, (char*)&range, sizeof(range), 0); // Send task

    // Receive result
    int sum;
    recv(masterSocket, (char*)&sum, sizeof(sum), 0);
    std::cout << "Sum from " << start << " to " << end << " = " << sum << std::endl;

    closesocket(masterSocket);
    WSACleanup();
    return 0;
}
