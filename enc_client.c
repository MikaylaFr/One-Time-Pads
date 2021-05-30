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

char NAME[12] = "enc_client\n";
char SERV_NAME[12] = "enc_server\n";

//Send message to server
void send_message(int connection_socket, char *message){
    char buffer[256];
    int charsRead = 0;
    char *saveptr = &message[0];
    int messageLen = strlen(message);

    //Determine how many times to call send
    int loop = messageLen / 255;
    int extra = strlen(message) % 255;

    if(loop > 0){
        for(int i=0; i < loop; i++){
            memset(buffer, '\0', 256);
            memcpy(buffer, saveptr, 255);
            charsRead = send(connection_socket, buffer, 255, 0);
            if (charsRead < 0){
                fprintf(stderr, "SERVER ERROR writing to socket\n");
            }
            //Move save ptr
            saveptr += 256;
        }
    }
    if(extra > 0){
        memset(buffer, '\0', 256);
        memcpy(buffer, saveptr, extra);
        charsRead = send(connection_socket, buffer, 255, 0);
        if (charsRead < 0){
          fprintf(stderr, "SERVER ERROR writing to socket\n");
        }
    }
    return;
}

//Receive message from server
void recv_message(int connection_socket, char *message){
    char buffer[256];
    const char term_char = '\n';
    int charsRead = 0;
    while(1){
        memset(buffer, '\0', 256);
        charsRead = recv(connection_socket, buffer, 255, 0);
        if (charsRead < 0){
            fprintf(stderr, "ERROR reading from socket\n");
            exit(1);
        }
        strcat(message, buffer);
        //Search for terminating character
        printf("searching for char\n");
        if(strchr(buffer, term_char) != NULL) break;
    }
    return;
}

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
    int socketFD;
    struct sockaddr_in serverAddress;
    char hostName[10] = "localhost";
    char buffer[256];
    memset(buffer, '\0', 256);

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
        fprintf(stderr, "CLIENT: ERROR connecting on port %d\n", listening_port);
        exit(1);
    }

    //Verify connection to enc_server
    char message[70500];
    memset(message, '\0', 70500);
    strcpy(message, NAME);
    send_message(socketFD, message);
    memset(message, '\0', 70500);
    //Verify connection
    recv_message(socketFD, message);
    if(strcmp(message, SERV_NAME) != 0){
        fprintf(stderr, "CLIENT: Not acceptable server, terminating");
        close(socketFD);
        exit(2);
    }
    //if not enc_serv, reject connection
    if(strcmp(message, SERV_NAME) != 0){
        fprintf(stderr, "CLIENT: Not designated server, rejecting connection\n");
        exit(2);
    }
    printf("CLIENT accepted connection");
    

    // Close the socket
    close(socketFD); 
    return;
}

/*
Check for bad chars
*/
int check_chars(char *str_to_check, int length){
    int curr_char = -1;
    for(int i=0; i < length-1; i++){
        curr_char = str_to_check[i];
        //check if within ascii range
        if(curr_char < 65 || curr_char > 90){
            //If not space,  or newline, stderr and exit
            if(curr_char != 32 && curr_char != 10){
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