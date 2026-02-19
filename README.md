# IPC client/server (C)

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)
![Docker: Ready](https://img.shields.io/badge/Infrastructure-Docker%20%2F%20Compose-blue?style=flat-square&logo=docker)
![Security: Non-Root](https://img.shields.io/badge/Security-Non--Root%20Container-green?style=flat-square)

A multi-threaded **client-server system in C** (Standard C11) designed to calculate the **minimum or maximum** of integers from a file. The system utilizes **POSIX Shared Memory** for low-latency communication and **POSIX Semaphores** for process synchronization, featuring a non-blocking command interface and concurrent request handling.

## Tech Stack & Infrastructure

* **Language:** C (POSIX threads, semaphores, shared memory).
* **IPC & Synchronization:** `shm_open`, `mmap`, `sem_t`, `pthread_mutex`.
* **Infrastructure:** Docker (Multi-stage build), Docker Compose.
* **Platform:** Debian-based production environment (Non-root user).

## DevOps & Containerization Features

* **Optimized Multi-stage Build:** The `Dockerfile` separates the compilation environment (GCC) from the lightweight runtime environment (Debian-slim), significantly reducing image size and the attack surface.
* **Shared Memory Orchestration:** Configured Docker Compose with `ipc: host` to allow seamless POSIX Shared Memory and Semaphore communication between isolated containers.
* **Security-First Approach:** The container is configured to run as a non-privileged `cuser`, adhering to the principle of least privilege.
* **Infrastructure as Code:** Fully automated deployment using Docker Compose, ensuring a consistent environment across different developer machines.

## System Features

* **Shared Memory Communication:** All data is passed via `mmap` shared memory segments, ensuring high performance.
* **Non-blocking Interface:** An asynchronous UI thread handles commands (`stat`, `reset`, `quit`) without interrupting active calculations.
* **Timed Access Control:** Clients implement a 5-second timeout (`sem_timedwait`) when the server is at maximum capacity.
* **Automated Reporting:** The server provides statistics on processed requests every 10 seconds.
* **Resource Integrity:** Guaranteed cleanup of all SHM and semaphore resources upon termination.



## Quick Start - Docker (Recommended)

The easiest way to run the system without local dependencies:

1.  **Build and start the server:**
    ```sh
    docker compose up -d server
    ```
2.  **Run the client:**
    ```sh
    docker compose run client
    ```
3.  **Interact with the server console:**
    ```sh
    docker attach c-shm-server
    ```

## Local Development - Manual Build

### Build
Use the provided `Makefile`:
```sh
make all
```

This compiles both the server and client.

Run Server
```sh
./server
```

Run Client
```sh
./client <filename> <operation>
```

Example:

``` sh
./client data.txt min
```


Where <operation> can be min or max.

### Local Development & Testing

Prepare a text file with space-separated integers (e.g., data.txt).

Start the server, run the client with the input file and desired operation.

Observe results on the client console and statistics on the server console.
