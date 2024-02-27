#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

mutex mtx;

bool check_prime(int n) {
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void thread_func(int lowerLimit, int upperLimit, vector<int>* primes) {
    for (int current_num = lowerLimit; current_num <= upperLimit; current_num++) {
        if (check_prime(current_num)) {
            lock_guard<mutex> lock(mtx); // Synchronize access to primes
            primes->push_back(current_num);
        }
    }
}

int processRange(int lowerLimit, int upperLimit) {
    vector<int> primes;
    int threadCount = thread::hardware_concurrency();
    vector<thread> threads;

    int rangeLength = upperLimit - lowerLimit + 1;
    int split = max(1, rangeLength / threadCount); // Ensure split is at least 1

    for (int i = 0; i < threadCount && lowerLimit + i * split <= upperLimit; ++i) {
        int start = lowerLimit + i * split;
        int end = min(upperLimit, start + split - 1);
        threads.emplace_back(thread(thread_func, start, end, &primes));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return primes.size();
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Could not create socket : " << WSAGetLastError() << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); // Ensure the port matches the master

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed with error code : " << WSAGetLastError() << endl;
        return 1;
    }

    listen(serverSocket, 3);
    cout << "Waiting for connections..." << endl;

    int c = sizeof(struct sockaddr_in);

    sockaddr_in clientAddr;

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &c);

    if (clientSocket == INVALID_SOCKET) {
        cerr << "Accept failed with error code : " << WSAGetLastError() << endl;
        return 1;
    }

    cout << "Connection accepted" << endl;

    int range[2];

    recv(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    // Print the range received from the master
    cout << "Received range: " << range[0] << " to " << range[1] << endl;

    // Sending back the count of primes found
    int primesCount = processRange(range[0], range[1]);
    send(clientSocket, reinterpret_cast<char*>(&primesCount), sizeof(primesCount), 0);

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
