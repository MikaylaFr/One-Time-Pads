/*
    File: enc_client.c
    Checks for errors before sending to server encryption. 
    Resources: https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Check for bad chars
*/
int check_chars(char *str_to_check, int length){
    int curr_char = -1;
    //printf("String: %s\nLength: %d\n", str_to_check, length);
    for(int i=0; i < length-1; i++){
        curr_char = str_to_check[i];
        //printf("Curr char: %d   %c\n", curr_char, curr_char);
        //check if within ascii range
        if(curr_char < 65 || curr_char > 90){
            //If not space,  or newline, stderr and exit
            if(curr_char != 32 && curr_char != 10){
                //printf("Bad: %c %d", curr_char, curr_char);
                return 1;
            }
        }
    }
    //if more than 1 newline
    return 0;
}

/*
Read from file and save to array
Checks for bad chars and if key file length
*/
char **process_files(char *file1, char *file2, char **enc_files){
    //Attempt open file
    FILE *plaintext_file = fopen(file1, "r");
    if(plaintext_file == NULL){
        fprintf(stderr, "Could not open plaintext file.\n");
        exit(3);
    };
    
    //Read and getline from plaintext file
    char *read_file = NULL;
    size_t length = 0; //Length of file
    size_t len = 0;
    //Read from plain text
    length = getline(&read_file, &len, plaintext_file);
    //Check for bad chars
    if(check_chars(read_file, length) > 0){
        fprintf(stderr, "Bad character in plaintext file\n");
        fclose(plaintext_file);
        exit (1);
    }
    //Copy over
    enc_files[0] = calloc(len, sizeof(char));
    strcpy(enc_files[0], read_file);
    fclose(plaintext_file);

    //Read and getline from key file
    FILE *key_file = fopen(file2, "r");
    if(plaintext_file == NULL){
        fprintf(stderr, "Could not open key file.\n");
        exit(3);
    };
    length = getline(&read_file, &len, key_file);
    //Check for bad chars
    if(check_chars(read_file, length) > 0){
        fprintf(stderr, "Bad character in key file\n");
        fclose(key_file);
        exit (1);
    }
    fclose(key_file);
    enc_files[1] = calloc(len, sizeof(char));
    strcpy(enc_files[1], read_file);
    
    free(read_file);
    return enc_files;
}

/*
enc_client plaintext key port
*/
int main(int argc, char *argv[]){
    char *enc_files[2];
    char **enc_strs = process_files(argv[1], argv[2], enc_files);
    printf("%s", enc_strs[0]);
    printf("%s", enc_strs[1]);

    free(enc_files[0]);
    free(enc_files[1]);
    return 0;
}