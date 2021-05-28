/*
    File: keygen.c
    Creates key for encryption and decryption
    Resources: https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/*
A-Z (0-25), ' '(26)
*/
int main(int argc, char *argv[]){
    /* Intializes random number generator */
    time_t t;
    srand((unsigned) time(&t));
    //Convert to int
    int keylength = atoi(argv[1]);
    //Output random capital characters
    for(int i=0; i < keylength; i++){
        int newNum = rand() % 27;
        if(newNum == 26) printf(" ");
        else printf("%c", newNum + 65);
    }
    //End with new line
    printf("\n");

    return 0;
}