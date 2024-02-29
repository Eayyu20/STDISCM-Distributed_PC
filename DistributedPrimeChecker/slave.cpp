//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <iostream>
//#include <winsock2.h>
//#include <vector>
//#include <thread>
//#include <mutex>
//
//using namespace std;
//
//#pragma comment(lib, "ws2_32.lib")
//
//mutex mtx;
//
//bool check_prime(int n) {
//    for (int i = 2; i * i <= n; i++) {
//        if (n % i == 0) return false;
//    }
//    return true;
//}
//
//void thread_func(int lowerLimit, int upperLimit, vector<int>* primes) {
//    for (int current_num = lowerLimit; current_num <= upperLimit; current_num++) {
//        if (check_prime(current_num)) {
//            lock_guard<mutex> lock(mtx); // Synchronize access to primes
//            primes->push_back(current_num);
//        }
//    }
//}
//
//vector<int> processRange(int lowerLimit, int upperLimit, int threadCount) {
//    vector<int> primes;
//    vector<thread> threads;
//
//    threads.reserve(threadCount);
//
//    int rangeLength = upperLimit - lowerLimit + 1;
//    int split = max(1, rangeLength / threadCount); // Ensure split is at least 1
//
//    for (int i = 0; i < threadCount && lowerLimit + i * split <= upperLimit; ++i) {
//        int start = lowerLimit + i * split;
//        int end = min(upperLimit, start + split - 1);
//        threads.emplace_back(thread(thread_func, start, end, &primes));
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    return primes;
//}
//
//// Serialization function
//void sendSerializedPrimes(SOCKET clientSocket, const vector<int>& primes) {
//    size_t primesCount = primes.size();
//    size_t bufferSize = sizeof(int) * primesCount;
//    vector<char> buffer(bufferSize); // Create a buffer for the serialized data
//
//    // Serialize the primes into the buffer
//    for (size_t i = 0; i < primesCount; ++i) {
//        int primeNetworkOrder = htonl(primes[i]);
//        memcpy(&buffer[i * sizeof(int)], &primeNetworkOrder, sizeof(int));
//    }
//
//    // Send the size of the primes array first
//    size_t primesCountNetwork = htonl(primesCount);
//    send(clientSocket, (char*)&primesCountNetwork, sizeof(primesCountNetwork), 0);
//
//    // Then, send the entire buffer
//    send(clientSocket, buffer.data(), bufferSize, 0);
//}
//
//int main() {
//    // Initialize Winsock
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        std::cerr << "WSAStartup failed." << std::endl;
//        return 1;
//    }
//
//    // Create socket
//    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if (clientSocket == INVALID_SOCKET) {
//        std::cerr << "Socket creation failed." << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // Connect to the server
//    sockaddr_in serverAddr;
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.37"); // Change to the server's IP address
//    serverAddr.sin_port = htons(12345);
//
//    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
//        std::cerr << "Connection failed." << std::endl;
//        closesocket(clientSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    std::cout << "Connected to server." << std::endl;
//
//    int receivedNumbers[3]; // Array to store received numbers
//    int bytesReceived = recv(clientSocket, (char*)receivedNumbers, sizeof(receivedNumbers), 0);
//    if (bytesReceived > 0) {
//        std::cout << "Received numbers: " << receivedNumbers[0] << ", " << receivedNumbers[1] << ", " << receivedNumbers[2] << std::endl;
//    }
//
//    vector<int> primes = processRange(receivedNumbers[0], receivedNumbers[1], receivedNumbers[2]);
//
//    // print primes
//    for (int i = 0; i < primes.size(); i++) {
//		cout << primes[i] << " ";
//	}
//
//    sendSerializedPrimes(clientSocket, primes);
//
//    // Close socket
//    closesocket(clientSocket);
//
//    // Cleanup Winsock
//    WSACleanup();
//
//    return 0;
//}
