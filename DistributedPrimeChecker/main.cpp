#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>
using namespace std;

#define LIMIT 10000000

mutex mtx;

/*
This function checks if an integer n is prime.

Parameters:
n : int - integer to check

Returns true if n is prime, and false otherwise.
*/
bool check_prime(const int& n);
void thread_func(int lowerLimit, int upperLimit, vector<int>* primes);
bool isPowerOfTwo(int n);

int main() {
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

    //start timer
    start = clock();

    threads.reserve(threadCount);

    int split = limit / threadCount;

    for (int i = 2; i <= limit; i = i + split + 1) {
        if (i + split > limit) {
            threads.emplace_back(thread(thread_func, i, limit, &primes));
        }
        else {
            threads.emplace_back(thread(thread_func, i, i + split, &primes));
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    end = clock();

    std::cout << primes.size() << " primes were found." << std::endl;

    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    cout << "Time taken by program is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;

    return 0;
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

bool check_prime(const int& n) {
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
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