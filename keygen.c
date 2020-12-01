/*Rowan Simmons*/
/*keygen.c*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]){
    srand(time(NULL));
    if (!argv[1]){ /*need to add key length*/
        fprintf(stderr, "%s", "keygen error: need to supply number. "
                              "example: [./keygen 10 > file]\n");
        exit(0);
    }

    int num = atoi(argv[1]); /*specified length*/
    int i;
    for (i = 0; i < num; i++) {
        /*27 allowed characters: 26 capital letters & space character*/
        char rand_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[rand() % 27];
        printf("%c", rand_char);
    }
    printf("\n");
    return 0;
}