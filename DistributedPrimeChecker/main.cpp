#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>

using namespace std;

mutex mtx;

#pragma comment(lib, "ws2_32.lib")

bool check_prime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void thread_func(int lowerLimit, int upperLimit, vector<int>* primes) {
    for (int current_num = lowerLimit; current_num <= upperLimit; current_num++) {
        if (check_prime(current_num)) {
            lock_guard<mutex> lock(mtx); // Use lock_guard for exception-safe locking
            primes->push_back(current_num);
        }
    }
}

void processClient(SOCKET clientSocket) {
    int range[2];
    recv(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    vector<int> primes;
    int threadCount = thread::hardware_concurrency(); // Use hardware concurrency for thread count
    vector<thread> threads;
    threads.reserve(threadCount);

    int lowerLimit = range[0];
    int upperLimit = range[1];
    int rangeLength = upperLimit - lowerLimit + 1;
    int split = rangeLength / threadCount + (rangeLength % threadCount != 0); // Ensure even distribution

    for (int i = 0; i < threadCount; ++i) {
        int start = lowerLimit + i * split;
        int end = i == threadCount - 1 ? upperLimit : start + split - 1;
        threads.emplace_back(thread(thread_func, start, end, &primes));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int primesCount = primes.size();
    send(clientSocket, reinterpret_cast<char*>(&primesCount), sizeof(primesCount), 0);

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Winsock init failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    listen(serverSocket, 5);

    cout << "Master server waiting for connections...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed with error: " << WSAGetLastError() << '\n';
            continue;
        }
        cout << "Master server waiting for connections...\n";
        thread clientThread(processClient, clientSocket);
    }

    WSACleanup();
    return 0;
}
