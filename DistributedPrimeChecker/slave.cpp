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
    cout << "Thread processing range: " << lowerLimit << " to " << upperLimit << endl;
    for (int current_num = lowerLimit; current_num <= upperLimit; current_num++) {
        if (check_prime(current_num)) {
            lock_guard<mutex> lock(mtx); // Synchronize access to primes
            primes->push_back(current_num);
            cout << "Found prime: " << current_num << endl; // Debug print
        }
    }
}


int processMaster(int upperLimit, int lowerLimit) {
    vector<int> primes;
    int threadCount = thread::hardware_concurrency(); // Use hardware concurrency for thread count
    vector<thread> threads;
    threads.reserve(threadCount);

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

    int primesCount = primes.size();

    cout << "Number of primes: " << primesCount << '\n';

    return primesCount;
}

int main() {
    WSADATA wsaData;
    SOCKET slaveSocket, masterSocket;
    sockaddr_in slaveAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    slaveSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (slaveSocket == INVALID_SOCKET) {
        cerr << "Could not create socket : " << WSAGetLastError() << endl;
        return 1;
    }

    slaveAddr.sin_family = AF_INET;
    slaveAddr.sin_addr.s_addr = INADDR_ANY;
    slaveAddr.sin_port = htons(12346);  // Use a different port than the master/client

    if (bind(slaveSocket, (struct sockaddr*)&slaveAddr, sizeof(slaveAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed with error code : " << WSAGetLastError() << endl;
        return 1;
    }

    listen(slaveSocket, 3);
    cout << "Waiting for connections..." << endl;

    int c = sizeof(struct sockaddr_in);
    sockaddr_in clientAddr;
    masterSocket = accept(slaveSocket, (struct sockaddr*)&clientAddr, &c);
    if (masterSocket == INVALID_SOCKET) {
        cerr << "Accept failed with error code : " << WSAGetLastError() << endl;
        return 1;
    }

    cout << "Connection accepted" << endl;
    int range[2];
    recv(masterSocket, reinterpret_cast<char*>(range), sizeof(range), 0);

    // Sending back the count of primes found
    int primesCount = processMaster(range[0], range[1]);
    send(masterSocket, reinterpret_cast<char*>(&primesCount), sizeof(primesCount), 0);

    closesocket(masterSocket);
    WSACleanup();
    return 0;
}
