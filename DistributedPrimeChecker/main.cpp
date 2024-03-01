#include <iostream>
#include <winsock2.h>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;
mutex mtx;
#define LIMIT 100000000
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

void thread_func(const vector<int>& numbers, size_t startIdx, size_t endIdx, vector<int>& primes) {
    for (size_t i = startIdx; i <= endIdx; i++) {
        if (check_prime(numbers[i])) {
            lock_guard<mutex> lock(mtx);
            primes.push_back(numbers[i]);
        }
    }
}

// Serialization function
void sendSerializedPrimes(SOCKET clientSocket, const vector<int>& primes) {
    size_t primesCount = primes.size();
    size_t bufferSize = sizeof(int) * primesCount;
    vector<char> buffer(bufferSize); // Create a buffer for the serialized data

    // Serialize the primes into the buffer
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
        totalBytesSent += bytesSent;
    }
}

// Deserialization function
vector<int> receiveSerializedPrimes(SOCKET clientSocket) {
    size_t primesCountNetwork;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&primesCountNetwork), sizeof(primesCountNetwork), 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive primes count." << endl;
        return vector<int>(); // Return an empty vector in case of error
    }
    size_t primesCount = ntohl(primesCountNetwork) + 1;

    vector<int> primes(primesCount);
    size_t totalBytesExpected = primesCount * sizeof(int);
    char* primesBuffer = reinterpret_cast<char*>(primes.data());

    size_t totalBytesReceived = 0;
    while (totalBytesReceived < totalBytesExpected) {
        bytesReceived = recv(clientSocket, primesBuffer + totalBytesReceived, totalBytesExpected - totalBytesReceived, 0);
        if (bytesReceived > 0) {
            totalBytesReceived += bytesReceived;
        }
        else if (bytesReceived == 0) {
            cerr << "Connection closed unexpectedly." << endl;
            break;
        }
        else {
            cerr << "recv failed: " << WSAGetLastError() << endl;
            return vector<int>(); // Return an empty vector in case of error
        }
    }

    for (size_t i = 0; i < primesCount; ++i) {
        primes[i] = ntohl(primes[i]); // Convert each prime back from network byte order
    }

    return primes;
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
    vector<int> masterPrimes;
    vector<int> slavePrimes;
    vector<thread> threads;
    int upperLimit = LIMIT;
    int lowerLimit = 2;
    int threadCount = 1;
    clock_t start, end;
    bool useSlave = false;
    char userInput;

    // Ask the user if they want to use a distributed model
    cout << "Do you want to run with a distributed model? (y/n): ";
    cin >> userInput;
    userInput = tolower(userInput);

    while (userInput != 'y' && userInput != 'n') {
        cout << "Invalid input. Please enter 'y' for yes or 'n' for no: ";
        cin >> userInput;
        userInput = tolower(userInput);
    }

    if (userInput == 'y') {
        useSlave = 1;
    }

    do {
        cout << "Enter lower bound (must be greater than or equal to 1): ";
        cin >> lowerLimit;

        if (lowerLimit < 1) {
            cout << "Error: Please enter a number greater than or equal to 1.\n";
        }
        else if (lowerLimit > LIMIT) {
            cout << "Error: Please enter a number less than or equal to 10000000.\n";
        }

    } while (lowerLimit < 1 || lowerLimit > LIMIT);

    do {
        cout << "Enter upper bound (must be greater than the lower bound): ";
        cin >> upperLimit;

        if (upperLimit < lowerLimit) {
            cout << "Error: Please enter a number greater than the lower bound.\n";
        }
        else if (upperLimit > LIMIT) {
            cout << "Error: Please enter a number less than or equal to 10000000.\n";
        }

    } while (upperLimit < 2 || upperLimit > LIMIT);

    do {
        cout << "Enter number of threads (must be a power of 2): ";
        cin >> threadCount;
    
        if (!isPowerOfTwo(threadCount)) {
            cout << "Error: Please enter a number that is a power of 2.\n";
        }
    } while (!isPowerOfTwo(threadCount));

    bool turn = true;
    //Remove even numbers from the range and sort them into masterPrimes and slavePrimes
    for (int i = lowerLimit; i <= upperLimit; i++) {
        if (!useSlave) {
            if (i % 2 != 0) {
                masterPrimes.push_back(i);
            }
        }
        else {
            if (i % 2 != 0) {
                if (turn) {
                    masterPrimes.push_back(i);
                }
                else {
                    slavePrimes.push_back(i);
                }
                turn = !turn;
            }
        }
	}

    SOCKET clientSocket = INVALID_SOCKET; // Initialize to an invalid socket

    if (useSlave) {
        // Listen for incoming connections
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Waiting for a connection..." << std::endl;

        // Accept a client connection
        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Connection established." << std::endl;

        //Append threadCount to slave
        slavePrimes.push_back(threadCount);
        //Send data to slave
        sendSerializedPrimes(clientSocket, slavePrimes);
    }

    //start timer
    start = clock();

    threads.reserve(threadCount);
    size_t totalNumbers = masterPrimes.size();
    size_t numbersPerThread = totalNumbers / threadCount;

    for (int i = 0; i < threadCount; ++i) {
        size_t startIdx = i * numbersPerThread;
        size_t endIdx = (i == threadCount - 1) ? totalNumbers - 1 : startIdx + numbersPerThread - 1;
        threads.emplace_back(thread_func, std::cref(masterPrimes), startIdx, endIdx, std::ref(primes));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (useSlave) {
        // Receive and deserialize the primes from the server
        std::vector<int> receivedPrimes = receiveSerializedPrimes(clientSocket);

        //Remove first element from receivedPrimes -> 0
        receivedPrimes.erase(receivedPrimes.begin());

        //print size of primes
        cout << "Primes received Count: " << receivedPrimes.size() << endl;

        //Merge the primes 
        primes.insert(primes.end(), receivedPrimes.begin(), receivedPrimes.end());
    }
 
    //Print prime size
    cout << "Primes Count: " << primes.size() << endl;

    // stop timer
    end = clock();

    //Calculate Time Taken
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    cout << "Time taken by program is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;

    // Close sockets
    if (useSlave) {
		closesocket(clientSocket);
	}

    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
