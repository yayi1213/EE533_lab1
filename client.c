
//   1) Creates a TCP socket
//   2) Resolves hostname -> IP address
//   3) Connects to the server
//   4) Reads a line from stdin, sends it to server
//   5) Receives a reply from server and prints it
//   6) Closes the socket

#include <stdio.h>      // printf, fprintf, perror
#include <stdlib.h>     // exit, atoi
#include <string.h>     // strlen
#include <strings.h>    // bzero, bcopy (BSD-style; sometimes discouraged but common in teaching code)
#include <unistd.h>     // read, write, close

#include <sys/types.h>  // basic system data types
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in, htons()
#include <netdb.h>      // gethostbyname(), struct hostent

// Print an error message (based on errno) and terminate the program.
// Using exit(1) means "abnormal termination / error occurred".
static void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd;   // file descriptor for the socket
    int portno;   // server port number
    int n;        // number of bytes read/written

    // serv_addr holds the server address information (IPv4 + port).
    struct sockaddr_in serv_addr;

    // server holds the result of DNS lookup (hostname -> IP address).
    struct hostent *server;

    // buffer for sending/receiving data.
    char buffer[256];

    // ------------------------------------------------------------------------
    // 1) Check command-line arguments:
    //    The client expects TWO arguments: hostname and port.
    // ------------------------------------------------------------------------
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(1);
    }

    // ------------------------------------------------------------------------
    // 2) Convert the port argument (string) to an integer.
    // ------------------------------------------------------------------------
    portno = atoi(argv[2]);

    // ------------------------------------------------------------------------
    // 3) Create a socket:
    //    - AF_INET      : IPv4
    //    - SOCK_STREAM  : TCP (reliable byte-stream)
    //    - 0            : choose the default protocol for SOCK_STREAM (TCP)
    //    socket() returns a file descriptor. If it fails, it returns -1.
    // ------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // ------------------------------------------------------------------------
    // 4) Resolve hostname -> IP address using DNS/hosts database:
    //    gethostbyname() returns a pointer to a hostent struct if successful.
    //    If it fails, it returns NULL.
    // ------------------------------------------------------------------------
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    // ------------------------------------------------------------------------
    // 5) Fill in the server address structure (sockaddr_in):
    //    - bzero() clears the struct to avoid garbage values in unused fields.
    //    - sin_family must be AF_INET for IPv4.
    // ------------------------------------------------------------------------
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    // ------------------------------------------------------------------------
    // 5.1) Copy the resolved IP address into serv_addr.sin_addr.s_addr:
    //      server->h_addr points to the first IP address in the result.
    //      server->h_length indicates the length of the address in bytes.
    // ------------------------------------------------------------------------
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    // ------------------------------------------------------------------------
    // 5.2) Set the port number:
    //      htons() converts from host byte order (often little-endian) to
    //      network byte order (big-endian).
    // ------------------------------------------------------------------------
    serv_addr.sin_port = htons(portno);

    // ------------------------------------------------------------------------
    // 6) Connect to the server:
    //    connect() performs the TCP 3-way handshake with the server.
    //    If connect() fails, it returns -1.
    // ------------------------------------------------------------------------
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    // ------------------------------------------------------------------------
    // 7) Send a message to the server:
    //    - Read a line from stdin using fgets()
    //    - write() sends bytes through the connected TCP socket
    // ------------------------------------------------------------------------
    printf("Please enter the message: ");
    bzero(buffer, sizeof(buffer));           // clear buffer
    fgets(buffer, sizeof(buffer) - 1, stdin); // read up to 255 chars + '\0'

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        error("ERROR writing to socket");
    }

    // ------------------------------------------------------------------------
    // 8) Receive the server reply:
    //    read() will block until data arrives (or connection is closed).
    // ------------------------------------------------------------------------
    bzero(buffer, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        error("ERROR reading from socket");
    }

    // Print what we received from the server.
    printf("%s\n", buffer);

    // ------------------------------------------------------------------------
    // 9) Close the socket:
    //    Always close file descriptors to free OS resources and properly
    //    terminate the TCP connection.
    // ------------------------------------------------------------------------
    close(sockfd);

    return 0;
}
