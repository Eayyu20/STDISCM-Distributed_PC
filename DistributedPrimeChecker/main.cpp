#include <iostream>
#include <winsock2.h>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;
mutex mtx;
#define LIMIT 10000000
#pragma comment(lib, "ws2_32.lib")

bool check_prime(const int& n) {
        for (int i = 2; i * i <= n; i++) {
         if (n % i == 0) {
             return false;
         }
     }
     return true;
}

bool isPowerOfTwo(int n) {
     if (n <= 0 || n > 1025) {
         return false;
     }
     else if (n == 1) {
         return true;
     }
     return (n & (n - 1)) == 0;
}

void thread_func(int lowerLimit, int upperLimit, vector<int>* primes) {
    for (int current_num = lowerLimit; current_num <= upperLimit; current_num++) {
        if (check_prime(current_num)) {
            //insert mutex here
            mtx.lock();
            primes->push_back(current_num);
            //release lock
            mtx.unlock();
        }
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //Input Handling
    vector<int> primes;
    vector<thread> threads;
    int upperLimit = LIMIT;
    int lowerLimit = 2;
    int threadCount = 1;
    clock_t start, end;

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

    do {
        cout << "Enter number of threads (must be a power of 2): ";
        cin >> threadCount;
    
        if (!isPowerOfTwo(threadCount)) {
            cout << "Error: Please enter a number that is a power of 2.\n";
        }
    } while (!isPowerOfTwo(threadCount));

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    // Accept a client connection
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connection established." << std::endl;
    
    //lowerLimit for Slave
    int midPoint = (upperLimit + lowerLimit) / 2; //change 2 with number of slaveprocess + 1

    // Send data to the slave
    int Task[3] = {midPoint, upperLimit, threadCount};
    send(clientSocket, (char*)Task, sizeof(Task), 0);

    //start timer
    start = clock();

    int rangeLength = midPoint - lowerLimit + 1;
    int split = max(1, rangeLength / threadCount); // Ensure split is at least 1

    for (int i = 0; i < threadCount && lowerLimit + i * split <= upperLimit; ++i) {
        int start = lowerLimit + i * split;
        int end = min(upperLimit, start + split - 1);
        threads.emplace_back(thread(thread_func, start, end, &primes));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    //Receive data from slave
    int primeCountFromSlave = 0;
    int recvSize = recv(clientSocket, (char*)&primeCountFromSlave, sizeof(primeCountFromSlave), 0);
    if (recvSize > 0) {
        cout << "Received prime count from slave: " << primeCountFromSlave << endl;
    }
    else if (recvSize == 0) {
        cout << "Connection closed." << endl;
    }
    else {
        cerr << "recv failed with error: " << WSAGetLastError() << endl;
    }

    // stop timer
    end = clock();

    //Calculate Time Taken
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    cout << "Time taken by program is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;

    //Output
    std::cout << primes.size() + primeCountFromSlave << " primes were found." << std::endl;

    // Close sockets
    closesocket(clientSocket);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
