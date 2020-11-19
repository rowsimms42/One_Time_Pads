// gcc -std=gnu99 -o enc server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Error function used for reporting issues*/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

/* Function to encrypt text*/
void encrypt_message(char** arg){
    size_t len_p = 0;
    size_t len_k = 0;
    char* plain_line = NULL;
    char* key_line = NULL;
    FILE *key_file = fopen(arg[1], "r");
    FILE *plain_file = fopen(arg[0], "r");
    if (key_file == NULL || plain_file == NULL)
        error("ERROR");
    if (getline(&plain_line, &len_p, plain_file) == -1)
        error("ERROR");
    if (getline(&key_line, &len_k, key_file) == -1)
        error("ERROR");
    if (len_k < len_p)
        error("ERROR key file: line length");

    char arr[len_p];
    int i = 0;
    while(plain_line[i]!='\n'){
        int j = plain_line[i];
        int k = key_line[i];
        if ((j != 32 && j<65) || j>90)
            error("ERROR bad characters in plain file");
        if ((k != 32 && k<65) || k>90)
            error("ERROR bad characters in key file");
        if (j > 32 && j!=k)
            j = j - 65;
        if (k > 32 && j!=k)
            k = k - 65;
        int sum = j+k;
        if (sum > 25)
            sum = sum - 26;
        sum = sum+65;
        if (j == k)
            sum = j;
        arr[i] = sum;
        i++;
    }
    arg[2] = arr;
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
    int connectionSocket, charsRead;
    char buffer[256];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
        error("ERROR opening socket");

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    listen(listenSocket, 5); /*allow up to 5 connections */

    /*Accept a connection, blocking if one is not available until one connects*/
    while(1){
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0)
            error("ERROR on accept");
        printf("SERVER: Connected to client running at host %d port %d\n",
               ntohs(clientAddress.sin_addr.s_addr),
               ntohs(clientAddress.sin_port));
        memset(buffer, '\0', 256);
        /* Read the client's message from the socket*/
        charsRead = recv(connectionSocket, buffer, 255, 0);
        if (charsRead < 0)
            error("ERROR reading from socket");
        char* save_ptr = NULL;
        char** arg = calloc(sizeof(buffer), sizeof(char));
        char* token = NULL;
        token = strtok_r(buffer, " ", &save_ptr);
        arg[0] = token;
        token = strtok_r(NULL, " \n", &save_ptr);
        arg[1] = token;
        /*printf("SERVER: from client: %s %s \n", arg[0], arg[1]);*/

        encrypt_message(arg);
        memset(buffer, '\0', 256);
        strncpy(buffer, arg[2], strlen(arg[2])-1);

        /* Send message back to the client*/
        charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
        if (charsRead < 0)
            error("ERROR writing to socket");
        /* Close the connection socket for this client*/
        close(connectionSocket);
        free(arg);
    }
    /* Close the listening socket*/
    close(listenSocket);
    return 0;
}
