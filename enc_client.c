/*
    File: enc_client.c
    Checks for errors before sending to server encryption. 
    Resources: https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
                client example
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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
      fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
      exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, 
          hostInfo->h_addr_list[0],
          hostInfo->h_length);
}

void server_encryption(char **enc_strs, char *port){
    int listening_port = atoi(port);
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char hostName[10] = "localhost";
    char buffer[256] = "blah blah blah\n";

    //create socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD < 0){
        fprintf(stderr, "CLIENT: ERROR opening socket\n");
        exit(1);
    }
    // Set up the server address struct
    setupAddressStruct(&serverAddress, listening_port, hostName);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        fprintf(stderr, "CLIENT: ERROR connecting\n");
        exit(1);
    }

    charsWritten = send(socketFD, buffer, strlen(buffer), 0); 
    if (charsWritten < 0){
      fprintf(stderr,"CLIENT: ERROR writing to socket\n");
    }
    if (charsWritten < strlen(buffer)){
      fprintf(stderr,"CLIENT: WARNING: Not all data written to socket!\n");
    }


    //Get return message from server
    //Clear out buffer for reuse
    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    if (charsRead < 0){
      fprintf(stderr, "CLIENT: ERROR reading from socket\n");
    }
    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Close the socket
    close(socketFD); 
    return;
}

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
        free(read_file);
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
        free(read_file);
        exit (1);
    }
    fclose(key_file);
    enc_files[1] = calloc(len, sizeof(char));
    strcpy(enc_files[1], read_file);

    //check for sizes
    if(strlen(enc_files[0]) > strlen(enc_files[1])){
        free(read_file);
        fprintf(stderr, "Not enough characters in key to encrypt\n");
        exit(1);
    }

    free(read_file);
    return enc_files;
}

/*
enc_client plaintext key port
*/
int main(int argc, char *argv[]){
    char *enc_files[2];
    char **enc_strs = process_files(argv[1], argv[2], enc_files);

    server_encryption(enc_strs, argv[3]);

    //printf("%s", ciphertext);

    free(enc_files[0]);
    free(enc_files[1]);
    return 0;
}