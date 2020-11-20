#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
//gcc -std=gnu99 -o enc_c client.c
// enc_client myplaintext mykey 57171 > myciphertext

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        error("CLIENT: ERROR, no such host\n");
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[5000];

    if (argc < 4) {
        fprintf(stderr,"USAGE: %s file key port\n", argv[0]);
        exit(1);
    }
    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }
    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }
    memset(buffer, '\0', sizeof(buffer));
    char* arg = calloc(256, sizeof(char));
    strncat(arg, argv[1], strlen(argv[1]));
    strncat(arg, " ", 1);
    strncat(arg, argv[2], strlen(argv[2]));
    strncpy(buffer, arg, strlen(arg));
    // Write to the server
    charsWritten = send(socketFD, buffer, strlen(buffer), 0);

    if (charsWritten < 0){
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer)){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    printf("%s\n", buffer);
    free(arg);

    // Close the socket
    close(socketFD);
    return 0;
}