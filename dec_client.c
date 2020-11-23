#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> /* send(),recv()*/
#include <netdb.h>      /* gethostbyname()*/
/*gcc -std=gnu99 -o dec_client dec_client.c*/
/* dec_client plaintext1.txt mykey 57171 > myciphertext*/

/* Error function used for reporting issues*/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

/* Set up the address struct*/
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    /* Store the port number*/
    address->sin_port = htons(portNumber);
    /* Get the DNS entry for this host name*/
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        error("CLIENT: ERROR, no such host\n");
    }
    /* Copy the first IP address from the DNS entry to sin_addr.s_addr*/
    memcpy((char*) &address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[1050];
    char type[3];

    if (argc < 4) {
        fprintf(stderr,"USAGE: %s file key port\n", argv[0]);
        exit(1);
    }
    /* Create a socket*/
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
        error("CLIENT: ERROR opening socket");
    /* Set up the server address struct*/
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
    /* Connect to server*/
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
        error("CLIENT: ERROR connecting");
    /*send client key*/
    charsWritten = send(socketFD, "d", 3, 0);
    if (charsWritten < 0)
        error("CLIENT: ERROR writing to socket");
    memset(buffer, '\0', sizeof(buffer));
    char* arg = calloc(256, sizeof(char));
    strncat(arg, argv[1], strlen(argv[1]));
    strncat(arg, " ", 1);
    strncat(arg, argv[2], strlen(argv[2]));
    strncpy(buffer, arg, strlen(arg));

    /*write to server*/
    charsWritten = send(socketFD, buffer, strlen(buffer), 0);
    if (charsWritten < 0)
        error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(buffer))
        printf("CLIENT: WARNING: Not all data written to socket!\n");

    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(socketFD, buffer, sizeof(buffer), 0);
    if (charsRead < 0)
        error("CLIENT: ERROR reading from socket");
    char* ptr = buffer;
    int size = atoi(ptr);
    int p = 0;
    charsWritten = send(socketFD, "ok", 5, 0);

    if (size > 1000){
        charsRead = 0;
        while (p < size) {
            memset(buffer, '\0', sizeof(buffer));
            charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
            if (charsRead < 0)
                error("CLIENT: ERROR reading from socket");
            p = p + charsRead;
            printf("%s", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
    else{
        memset(buffer, '\0', sizeof(buffer));
        charsRead = recv(socketFD, buffer, sizeof(buffer), 0);
        if (charsRead < 0)
            error("CLIENT: ERROR reading from socket");
        printf("%s", buffer);

    }


    free(arg);
    /* Close the socket*/
    close(socketFD);
    return 0;
}