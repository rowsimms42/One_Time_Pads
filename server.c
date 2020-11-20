// gcc -std=gnu99 -o enc server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>

int status = 0;
/* Error function used for reporting issues*/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

void return_status(int wstatus) {
    if(WIFEXITED(wstatus)){
        printf("exit status: %d\n", WEXITSTATUS(wstatus));
    } else{
        printf("terminated by signal: %d\n", WTERMSIG(wstatus));
    }
    fflush(stdout);
}

void encrypt_message(char** arg, char* out){
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
        if (j == 32)
            j = 91;
        if (k == 32)
            k = 91;
        j = j - 65;
        k = k - 65;
        int sum = j+k;
        if (sum > 26)
            sum = sum - 27;
        sum = sum+65;
        if (sum == 91){
            sum = 0;
            sum = 32;
        }
        arr[i] = sum;
        i++;
    }
    arr[i] = '\n';
    strcpy(out, arr);
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
    char buffer[5000];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t spawnpid;

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
    while (1) {
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *) &clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0)
            error("ERROR on accept");

        spawnpid = fork();
        switch (spawnpid) {
            case 0:
                memset(buffer, '\0', 256);
                /* Read the client's message from the socket*/
                charsRead = recv(connectionSocket, buffer, 255, 0);
                if (charsRead < 0)
                    error("ERROR reading from socket");
                char *save_ptr = NULL;
                char **arg = calloc(sizeof(buffer), sizeof(char));
                char* out = calloc(5000, sizeof(char));
                char *token = NULL;
                token = strtok_r(buffer, " ", &save_ptr);
                arg[0] = token;
                token = strtok_r(NULL, " \n", &save_ptr);
                arg[1] = token;
                encrypt_message(arg, out);
                memset(buffer, '\0', 256);
                strncpy(buffer, out, strlen(out) - 1);
                free(arg);
                free(out);

                /* Send message back to the client*/
                charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
                if (charsRead < 0)
                    error("ERROR writing to socket");
                break;
            case -1:
                error("fork error");
                break;
            default:
                waitpid(spawnpid, &status, WNOHANG);
        }
        close(connectionSocket);
    }

    /* Close the listening socket*/
    close(listenSocket);
    return 0;
}
