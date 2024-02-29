#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

mutex mtx;

bool check_prime(int n) {
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void thread_func(const vector<int>& numbers, size_t startIdx, size_t endIdx, vector<int>& primes) {
    for (size_t i = startIdx; i <= endIdx; i++) {
        if (check_prime(numbers[i])) {
            lock_guard<mutex> lock(mtx);
            primes.push_back(numbers[i]);
        }
    }
}

vector<int> receiveArrayFromMaster(SOCKET clientSocket, int& threadCount) {
    // Receive the size of the array
    size_t dataArraySizeNetwork;
    recv(clientSocket, reinterpret_cast<char*>(&dataArraySizeNetwork), sizeof(dataArraySizeNetwork), 0);
    size_t dataArraySize = ntohl(dataArraySizeNetwork);
    cout << "Expected array size: " << dataArraySize << endl;

    // Receive the array elements
    vector<int> dataArray(dataArraySize - 1);
    for (size_t i = 0; i < dataArraySize; ++i) {
        int dataNetworkOrder;
        int recvResult = recv(clientSocket, reinterpret_cast<char*>(&dataNetworkOrder), sizeof(dataNetworkOrder), 0);
        if (recvResult == SOCKET_ERROR) {
            cerr << "Failed to receive data: " << WSAGetLastError() << endl;
            break;
        }
        if (i == dataArraySize - 1) {
			threadCount = ntohl(dataNetworkOrder);
		}
        else {
            dataArray[i] = ntohl(dataNetworkOrder);
        }
        
    }
    return dataArray;
}

vector<int> processArray(const vector<int>& numbers, int threadCount) {
    vector<int> primes;
    vector<thread> threads;
    size_t totalNumbers = numbers.size();

    threads.reserve(threadCount);

    size_t numbersPerThread = totalNumbers / threadCount;

    for (int i = 0; i < threadCount; ++i) {
        size_t startIdx = i * numbersPerThread;
        size_t endIdx = (i == threadCount - 1) ? totalNumbers - 1 : startIdx + numbersPerThread - 1;
        threads.emplace_back(thread_func, std::cref(numbers), startIdx, endIdx, std::ref(primes));
    }

    for (auto& t : threads) {
        t.join();
    }

    return primes;
}

// Serialization function
void sendSerializedPrimes(SOCKET clientSocket, const vector<int>& primes) {
    size_t primesCount = primes.size();
    size_t bufferSize = sizeof(int) * primesCount;
    vector<char> buffer(bufferSize); // Create a buffer for the serialized data

    // Serialize the primes into the buffer
    cout << "Serializing and Sending Primes..." << endl;
    for (size_t i = 0; i < primesCount; ++i) {
        int primeNetworkOrder = htonl(primes[i]);
        memcpy(&buffer[i * sizeof(int)], &primeNetworkOrder, sizeof(int));
    }

    // Send the size of the primes array first
    size_t primesCountNetwork = htonl(primesCount);
    int bytesSent = send(clientSocket, (char*)&primesCountNetwork, sizeof(primesCountNetwork), 0);
    if (bytesSent == SOCKET_ERROR) {
        cerr << "Failed to send primes count: " << WSAGetLastError() << endl;
        return;
    }

    // Then, send the entire buffer
    size_t totalBytesSent = 0;
    while (totalBytesSent < bufferSize) {
        bytesSent = send(clientSocket, buffer.data() + totalBytesSent, bufferSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            cerr << "Failed to send prime data: " << WSAGetLastError() << endl;
            return;
        }
        else {
            cout << "Sent " << bytesSent / 1024.0 << " KB." << endl;
        }
        totalBytesSent += bytesSent;
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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.37"); // Change to the server's IP address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    int threadCount = 0;

    vector<int> numbers = receiveArrayFromMaster(clientSocket, threadCount);

    cout << "Processing Array..." << endl;

    vector<int> primes = processArray(numbers, threadCount);

    // Print size of primes
    cout << "Primes count: " << primes.size() << endl;

    // Print primes (uncomment for debugging)
    /*for (int i = 0; i < primes.size(); i++) {
        cout << primes[i] << " ";
    }*/

    sendSerializedPrimes(clientSocket, primes);

    // Close socket
    closesocket(clientSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}