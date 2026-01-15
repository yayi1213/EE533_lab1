//   1) Creates a TCP listening socket
//   2) Binds the socket to a local port
//   3) Listens for incoming client connections
//   4) For each client connection:
//        - fork() a child process
//        - child handles client communication
//        - parent continues accepting new clients
//   5) Uses a SIGCHLD handler to prevent zombie processes

#include <stdio.h>      // printf, fprintf, perror
#include <stdlib.h>     // exit, atoi
#include <string.h>     // memset / bzero
#include <unistd.h>     // read, write, close, fork
#include <sys/types.h>  // system data types
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in, htons, INADDR_ANY
#include <signal.h>     // signal, SIGCHLD
#include <sys/wait.h>   // waitpid

// -----------------------------------------------------------------------------
// Error handling function:
// Prints an error message (based on errno) and terminates the program.
// -----------------------------------------------------------------------------
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// -----------------------------------------------------------------------------
// dostuff():
// Handles communication with a SINGLE client.
// This function runs in the CHILD process after fork().
//
// Steps:
//   1) Read data sent by the client
//   2) Print the received message
//   3) Send a response back to the client
// -----------------------------------------------------------------------------
void dostuff(int sockfd)
{
    char buffer[256];
    int n;

    // Clear buffer to avoid leftover data
    bzero(buffer, sizeof(buffer));

    // Read message from client
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0)
        error("ERROR reading from socket");

    printf("Message from client: %s\n", buffer);

    // Send response to client
    n = write(sockfd, "I got your message", 18);
    if (n < 0)
        error("ERROR writing to socket");
}

// -----------------------------------------------------------------------------
// SigCatcher():
// Signal handler for SIGCHLD.
// Called when a child process terminates.
//
// Purpose:
//   - Reap finished child processes
//   - Prevent zombie processes
//
// waitpid(-1, NULL, WNOHANG):
//   - -1     : wait for ANY child process
//   - NULL   : ignore exit status
//   - WNOHANG: do not block if no child has exited
// -----------------------------------------------------------------------------
void SigCatcher(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ; // reap all terminated children
}

int main(int argc, char *argv[])
{
    int sockfd;             // listening socket file descriptor
    int newsockfd;          // connected socket file descriptor
    int portno;             // port number
    socklen_t clilen;       // length of client address structure

    struct sockaddr_in serv_addr; // server address
    struct sockaddr_in cli_addr;  // client address

    // -------------------------------------------------------------------------
    // Install signal handler for SIGCHLD:
    // Ensures terminated child processes are cleaned up properly.
    // -------------------------------------------------------------------------
    signal(SIGCHLD, SigCatcher);

    // -------------------------------------------------------------------------
    // Check command-line arguments:
    // The server requires ONE argument: the port number.
    // -------------------------------------------------------------------------
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // -------------------------------------------------------------------------
    // Create a TCP socket:
    //   AF_INET     : IPv4
    //   SOCK_STREAM : TCP
    // -------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // -------------------------------------------------------------------------
    // Initialize server address structure:
    // Clear all fields to avoid garbage values.
    // -------------------------------------------------------------------------
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);              // convert port argument to integer

    serv_addr.sin_family = AF_INET;      // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // INADDR_ANY means the server accepts connections on ANY local IP
    serv_addr.sin_port = htons(portno);  // host byte order -> network byte order

    // -------------------------------------------------------------------------
    // Bind the socket to the specified IP address and port:
    // After bind(), the OS knows this socket is the server for this port.
    // -------------------------------------------------------------------------
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // -------------------------------------------------------------------------
    // Listen for incoming connections:
    // backlog = 5 means up to 5 pending connections can be queued.
    // -------------------------------------------------------------------------
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    // -------------------------------------------------------------------------
    // Main server loop:
    // Continuously accept and handle incoming client connections.
    // -------------------------------------------------------------------------
    while (1) {

        // accept() blocks until a client connects
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        // ---------------------------------------------------------------------
        // fork() creates a new process:
        //   - return value < 0 : error
        //   - return value = 0 : child process
        //   - return value > 0 : parent process
        // ---------------------------------------------------------------------
        pid_t pid = fork();
        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0) {
            // ---------------------- Child process ----------------------------
            // Child does NOT need the listening socket
            close(sockfd);

            // Handle client communication
            dostuff(newsockfd);

            // Close client socket after communication is done
            close(newsockfd);

            // Terminate child process
            exit(0);
        } else {
            // ---------------------- Parent process ---------------------------
            // Parent does NOT communicate with the client
            // Close the connected socket and continue accepting new clients
            close(newsockfd);
        }
    }

    return 0;
}