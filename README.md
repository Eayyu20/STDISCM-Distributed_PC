# Distributed Prime Number Calculator

This project demonstrates a distributed computing approach to calculate prime numbers within a specified range. It consists of two main components: a master application and a slave application. The master can operate independently or distribute part of its workload to the slave application over a network connection.

## Requirements

- Windows OS (due to the use of Winsock2 for networking).
- C++ compiler (e.g., GCC, MSVC)
- Network connectivity between the master and slave nodes.

## Setup

1. **Winsock2 Library**: Ensure that the Winsock2 library is accessible to your compiler. This project uses `winsock2.h`, which is typically available on Windows systems.

2. **Configuration**: Configure the IP address and port in the slave component to match the master's listening address and port. Ensure that both Master and Slave are connected to the same network.

3. **Compilation**: Compile both the master and slave components using your C++ compiler. For example, with g++:

```bash
g++ main.cpp -o main -lws2_32
g++ slave.cpp -o slave -lws2_32
``` 

## Running the Application

1. **Start the Master**: Run the main executable. When prompted, choose whether to run in standalone or distributed mode. Once a slave connects, the master can distribute the workload.

2. **Start the Slave**: Run the slave executable on one or more machines, ensuring they are configured to connect to the master's IP address and port.

3. **Input Parameters**: The master will prompt for:
   - Whether to run in distributed mode (with slaves) or standalone.
   - The lower and upper bounds of the range to search for prime numbers.
   - The number of threads to use for processing.

## Authors

* **Go, Eldrich**
* **Pangan, John**
* **Pinawin, Timothy**
* **Yu, Ethan**
