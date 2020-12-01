/*Rowan Simmons*/
/*dec_server.c*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int status = 0;
/* Error function used for reporting issues*/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

/*function that decrypts message. takes in argument with name of text file and key file*/
void decrypt_message(char** arg, char* out){
    ssize_t len_c;
    ssize_t len_k;
    size_t linecap_c = 0;
    size_t linecap_k = 0;
    char* cipher_line = NULL;
    char* key_line = NULL;
    /*open files*/
    FILE *key_file = fopen(arg[1], "r");
    FILE *cipher_file = fopen(arg[0], "r");
    if (key_file == NULL || cipher_file == NULL)
        error("ERROR");
    len_c = getline(&cipher_line, &linecap_c, cipher_file);
    len_k = getline(&key_line, &linecap_k, key_file);
    /*error check*/
    if (len_c == -1 || len_k == -1) {
        fprintf(stderr, "ERROR-NO FILE\n");
        exit(1);
    }
    if (len_k < len_c) {
        fprintf(stderr, "ERROR-KEY LENGTH IS TOO SHORT\n");
        exit(1);
    }

    char* arr = calloc(len_c, sizeof(char));
    int i=0;
    int j, k, dif;
    /*traverse file char by char*/
    while(cipher_line[i]!='\n'){
        j = cipher_line[i];
        k = key_line[i];
        if ((((j!=32 && j<65) || j > 90)) || (((k!=32 && k<65) || k > 90))) {
            fprintf(stderr, "ERROR bad characters\n");/*bad file*/
            exit(1);
            break;
        }
        if (j == 32)
            j = 91;
        if (k == 32)
            k = 91;
        j = j - 65;
        k = k - 65;
        dif = j-k;
        if (dif < 0)
            dif = dif + 27;
        dif = dif+65;
        if (dif == 91){
            dif = 0;
            dif = 32;
        }
        arr[i]= dif; /*fill array*/
        i++;
    }
    arr[i] = '\n'; /*add new line*/
    strncpy(out, arr, strlen(arr)); /*copy array to out*/
    free(arr);
    fclose(key_file);
    fclose(cipher_file);
}

/*initialize address struct*/
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

/*determine file size*/
int file_size(char** arg){
    FILE *file = fopen(arg[0], "r");
    size_t max = 0;
    char* line = NULL;
    ssize_t len = getline(&line, &max, file);
    char* out = calloc(len, sizeof(char));
    fclose(file);
    return len;
}

/*main function*/
int main(int argc, char *argv[]){
    int connectionSocket, charsRead;
    char buffer[1050];
    char type[3];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t spawnpid;

    /*need to add port number to start server*/
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
        error("ERROR opening socket");

    /* Set up the address struct for the server socket*/
    setupAddressStruct(&serverAddress, atoi(argv[1]));
    /* Associate the socket to the port*/
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

        /*create new process*/
        spawnpid = fork();
        switch (spawnpid) {
            case 0:
                memset(type, '\0', sizeof(type));
                charsRead = recv(connectionSocket, type, sizeof(type), 0);
                if (charsRead < 0)
                    error("SERVER: ERROR reading from socket");
                if (strcmp(type, "d") != 0) {
                    fprintf(stderr, "ERROR: Client/Server type mismatch\n");
                    exit(1);
                }
                memset(type, '\0', sizeof(type));
                memset(buffer, '\0', sizeof(buffer));
                /* Read the client's message from the socket*/
                charsRead = recv(connectionSocket, buffer, 255, 0);
                if (charsRead < 0)
                    error("ERROR reading from socket");
                char* save_ptr = NULL;
                char** arg = calloc(sizeof(buffer), sizeof(char));
                /*parse message and store in arg*/
                char* token = NULL;
                token = strtok_r(buffer, " ", &save_ptr);
                arg[0] = token;
                token = strtok_r(NULL, " \n", &save_ptr);
                arg[1] = token;
                int size = file_size(arg);
                char* out = calloc(size, sizeof(char)+1);
                /*decrypt message function*/
                decrypt_message(arg, out);
                char* str = out;
                int s = size;
                int n = 0;
                char to_str[10];
                /*int->string then send back to client*/
                sprintf(to_str, "%d", size);
                strncpy(buffer, to_str, strlen(to_str));
                charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
                /*clear buffer then read message from client*/
                memset(buffer, 0, sizeof(buffer));
                charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);

                if (charsRead < 0)
                    error("ERROR writing to socket");
                /*read up to 1000 characters at a time*/
                if (size > 1000){
                    while (s > 0) {
                        if (s >= 1000)
                            n = 1000;
                        else
                            n = s;
                        memset(buffer, '\0', sizeof(buffer));
                        strncpy(buffer, out, n);
                        /*send to client*/
                        charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
                        if (charsRead < 0) {
                            error("ERROR writing to socket");
                            break;
                        }
                        out = out + 1000; /*offset*/
                        s = s-1000;
                    }
                }
                else{ /*message is less than 1000 chars*/
                    memset(buffer, '\0', sizeof(buffer));
                    strncpy(buffer, out, strlen(str));
                    charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
                    if (charsRead < 0) {
                        error("ERROR writing to socket");
                        break;
                    }
                }
                free(arg);
                free(str);
                break;
            case -1:
                error("fork error");
                break;
            default:
                waitpid(spawnpid, &status, WNOHANG);
        }
        close(connectionSocket); /*parent closes client socket*/
    }

    /* Close the listening socket*/
    close(listenSocket);
    return 0;
}
