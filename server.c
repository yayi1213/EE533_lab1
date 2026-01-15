//   1) Creates a TCP socket
//   2) Binds the socket to a local port
//   3) Listens for incoming connections
//   4) Accepts ONE client connection
//   5) Reads data sent by the client
//   6) Sends a reply back to the client
//   7) Closes the connection and exits

#include <stdio.h>      // printf, fprintf, perror
#include <stdlib.h>     // exit, atoi
#include <string.h>     // strlen, memset
#include <unistd.h>     // read, write, close

#include <sys/types.h>  // basic system data types
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <netinet/in.h> // struct sockaddr_in, htons(), INADDR_ANY

// Print an error message (based on errno) and terminate the program.
static void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd;         // listening socket file descriptor
    int newsockfd;      // connected socket file descriptor
    int portno;         // port number
    socklen_t clilen;   // length of client address structure
    int n;              // number of bytes read/written

    char buffer[256];   // buffer for receiving/sending data

    struct sockaddr_in serv_addr; // server address
    struct sockaddr_in cli_addr;  // client address

    // ------------------------------------------------------------------------
    // 1) Check command-line arguments:
    //    The server requires ONE argument: the port number.
    // ------------------------------------------------------------------------
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // ------------------------------------------------------------------------
    // 2) Create a socket:
    //    - AF_INET     : IPv4
    //    - SOCK_STREAM : TCP
    // ------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // ------------------------------------------------------------------------
    // 3) Initialize the server address structure:
    //    memset() clears the structure to avoid garbage values.
    // ------------------------------------------------------------------------
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);              // convert port argument to integer

    serv_addr.sin_family = AF_INET;      // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // INADDR_ANY means the server accepts connections on ANY local IP address
    serv_addr.sin_port = htons(portno);  // host byte order -> network byte order

    // ------------------------------------------------------------------------
    // 4) Bind the socket to the specified IP address and port:
    //    After bind(), the OS knows which port this server listens on.
    // ------------------------------------------------------------------------
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    // ------------------------------------------------------------------------
    // 5) Listen for incoming connections:
    //    The second argument (5) is the backlog size (max pending connections).
    // ------------------------------------------------------------------------
    listen(sockfd, 5);

    // ------------------------------------------------------------------------
    // 6) Accept a client connection:
    //    accept() blocks until a client connects.
    //    It returns a NEW socket descriptor (newsockfd) for this connection.
    // ------------------------------------------------------------------------
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    // ------------------------------------------------------------------------
    // 7) Read data from the client:
    //    read() blocks until data is received.
    // ------------------------------------------------------------------------
    memset(buffer, 0, sizeof(buffer));
    n = read(newsockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        error("ERROR reading from socket");
    }

    printf("Here is the message: %s\n", buffer);

    // ------------------------------------------------------------------------
    // 8) Write a response back to the client:
    // ------------------------------------------------------------------------
    n = write(newsockfd, "I got your message", 18);
    if (n < 0) {
        error("ERROR writing to socket");
    }

    // ------------------------------------------------------------------------
    // 9) Close sockets:
    //    - Close the connected socket first
    //    - Then close the listening socket
    // ------------------------------------------------------------------------
    close(newsockfd);
    close(sockfd);

    return 0;
}