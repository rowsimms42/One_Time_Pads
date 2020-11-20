// gcc -std=gnu99 -o enc testing.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
}

void decrypt_message(char** arg, char* out){
    size_t len_c = 0;
    size_t len_k = 0;
    char* cipher_line = NULL;
    char* key_line = NULL;
    FILE *key_file = fopen(arg[1], "r");
    FILE *cipher_file = fopen(arg[0], "r");
    if (key_file == NULL || cipher_file == NULL)
        error("ERROR");
    if (getline(&cipher_line, &len_c, cipher_file) == -1)
        error("ERROR");
    if (getline(&key_line, &len_k, key_file) == -1)
        error("ERROR");
    if (len_k < len_c)
        error("ERROR key file: line length");

    char arr[len_c];

    int i = 0;
    int j = 0;
    int k = 0;
    int dif = 0;
    while(cipher_line[i]!='\n'){
        j = cipher_line[i];
        k = key_line[i];
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
        arr[i] = dif;
        i++;
    }
    arr[i] = '\n';
    strcpy(out, arr);
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


int main(int argc, char *argv[]) {
    char buffer[256];
    int i;
    char** arg = calloc(256, sizeof(char));
    char* out = calloc(5000, sizeof(char));

    arg[0] = argv[1];
    arg[1] = argv[2];

    //encrypt_message(arg, out);
    decrypt_message(arg, out);

    printf("%s", out);
    free(arg);
    free(out);

    return 0;
}