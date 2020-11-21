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
    ssize_t len_c;
    ssize_t len_k;
    size_t linecap_c = 0;
    size_t linecap_k = 0;
    char* cipher_line = NULL;
    char* key_line = NULL;
    FILE *key_file = fopen(arg[1], "r");
    FILE *cipher_file = fopen(arg[0], "r");
    if (key_file == NULL || cipher_file == NULL)
        error("ERROR");
    len_c = getline(&cipher_line, &linecap_c, cipher_file);
    len_k = getline(&key_line, &linecap_k, key_file);
    if (len_c == -1 || len_k == -1)
        error("ERROR");
    if (len_k < len_c)
        error("ERROR");

    char* arr = calloc(len_c, sizeof(char));
    int i=0;
    int j, k, dif;
    while(cipher_line[i]!='\n'){
        j = cipher_line[i];
        k = key_line[i];
        if ((((j!=32 && j<65) || j > 90)) || (((k!=32 && k<65) || k > 90)))
            error("ERROR bad characters");
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
        arr[i]= dif;
        i++;
    }
    strncpy(out, arr, strlen(arr));
    free(arr);
}


void encrypt_message(char** arg, char* out){
    ssize_t len_p;
    ssize_t len_k;
    size_t linecap_p = 0;
    size_t linecap_k = 0;
    char* plain_line = NULL;
    char* key_line = NULL;
    FILE *key_file = fopen(arg[1], "r");
    FILE *plain_file = fopen(arg[0], "r");
    if (key_file == NULL || plain_file == NULL)
        error("ERROR-NULL");
    len_p = getline(&plain_line, &linecap_p, plain_file);
    len_k = getline(&key_line, &linecap_k, key_file);
    if (len_p == -1 || len_k == -1)
        error("ERROR-NO FILE");
    if (len_k < len_p)
        error("ERROR-LINE LENGTH");

    char* arr = calloc(len_p, sizeof(char));
    int i=0;
    int j, k, sum;
    while(plain_line[i]!='\n'){
        int j = plain_line[i];
        int k = key_line[i];
        if (j == 32)
            j = 91;
        if (k == 32)
            k = 91;
        j = j - 65;
        k = k - 65;
        sum = j+k;
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
    strncpy(out, arr, strlen(arr));
    free(arr);
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

    printf("%s\n", out);
    free(arg);
    free(out);

    return 0;
}