# EE533_lab1
## 1. Objective

The objective of this lab is to implement a TCP client and a fork-based concurrent TCP server using the Berkeley socket API.
The server is designed to handle multiple client connections concurrently and properly prevent zombie processes.

## 2. Files

fork_server.c

Fork-based concurrent TCP server with SIGCHLD handler.

client.c

Simple TCP client for sending and receiving messages.

## 3. System Environment

Operating System: Linux (Ubuntu / VMware Virtual Platform)

Language: C

Compiler: gcc

## 4. Compilation
```
gcc server.c -o server
gcc client.c -o client
```

## 5. Execution
Start the Server
```
./server <port>
```

Example:
```
./server 5000
```
Run the Client
```
./client <hostname> <port>
```

Example:
```
./client localhost 5000
```
## 6. Server Design

This server uses a fork-based concurrency model:

The server creates a TCP listening socket.

It waits for incoming client connections using accept().

For each accepted connection, the server calls fork().

The child process handles communication with the client.

The parent process continues accepting new connections.

This design allows multiple clients to be served simultaneously.

## 7. Zombie Process Handling
#### Problem

When a child process terminates, it becomes a zombie process until the parent process collects its exit status.

#### Solution

A SIGCHLD signal handler is installed in the server:
```
signal(SIGCHLD, SigCatcher);

void SigCatcher(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
```

This ensures that all terminated child processes are properly reaped and no zombie processes remain.

## 8. Zombie Verification

To verify that no zombie processes exist, the following command was used:
```
ps -ef | grep defunct | grep -v grep
```
Result

No output was observed.

Conclusion

This confirms that zombie processes are successfully prevented by the SIGCHLD handler.

## 9. Key Concepts

TCP socket programming

Client-server architecture

fork()-based concurrency

Signal handling (SIGCHLD)

Zombie process prevention

Process lifecycle management

## 10. Conclusion

In this lab, a fork-based concurrent TCP server was successfully implemented.
By using process-based concurrency and proper signal handling, the server can handle multiple clients efficiently while avoiding zombie processes.