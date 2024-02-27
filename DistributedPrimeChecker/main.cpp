#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>

using namespace std;

mutex mtx;

#pragma comment(lib, "ws2_32.lib")

#define LIMIT 10000000

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

int processClient(int upperLimit, int lowerLimit) {

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

    int upperLimit = LIMIT;
    int lowerLimit = 2;

    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Could not create socket : " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("10.52.1.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connect failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    do {
        cout << "Enter lower bound (must be greater than or equal to 2): ";
        cin >> lowerLimit;

        if (lowerLimit < 2) {
            cout << "Error: Please enter a number greater than or equal to 2.\n";
        }
        else if (lowerLimit > 10000000) {
            cout << "Error: Please enter a number less than or equal to 10000000.\n";
        }

    } while (lowerLimit < 2 || lowerLimit > 10000000);

    do {
        cout << "Enter upper bound (must be greater than or equal to lower bound): ";
        cin >> upperLimit;

        if (upperLimit < lowerLimit) {
            cout << "Error: Please enter a number greater than or equal to lower bound.\n";
        }
        else if (upperLimit > 10000000) {
            cout << "Error: Please enter a number less than or equal to 10000000.\n";
        }

    } while (upperLimit < 2 || upperLimit > 10000000);

    // Determine the midpoint of the range
    int midPoint = (upperLimit + lowerLimit) / 2;

    // Send the second half of the range to the slave for processing
    int range[2] = { midPoint + 1, upperLimit };
    if (send(clientSocket, reinterpret_cast<char*>(range), sizeof(range), 0) < 0) {
        cerr << "Failed to send range to slave: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1; // Make sure to return if sending fails
    }

    // Print the range sent to the slave
    cout << "Sent range to slave: " << range[0] << " to " << range[1] << endl;

    // Process the first half locally
    cout << "Processing range locally: " << lowerLimit << " to " << midPoint << endl;
    int localPrimes = processClient(lowerLimit, midPoint); // Assume this function now returns the count of primes

    // Wait for and receive the result from the slave
    int slavePrimes;
    if (recv(clientSocket, reinterpret_cast<char*>(&slavePrimes), sizeof(slavePrimes), 0) <= 0) {
        cerr << "Failed to receive results from slave: " << WSAGetLastError() << endl;
    }
    else {
        cout << "Slave found " << slavePrimes << " prime numbers in the range " << range[0] << " to " << range[1] << endl;
    }

    // Combine the results
    int totalPrimes = localPrimes + slavePrimes;
    cout << "Total number of primes found: " << totalPrimes << endl;

    WSACleanup();
    return 0;
}