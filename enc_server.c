/*
    File: enc_server.c
    Gets plaintext from socket connection, encrypts plain text, send back to client
    Resources: Example server program
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <math.h>

char NAME[12] = "enc_server\n";
char CLIENT_NAME[12] = "enc_client\n";

//Encryption function
char *encrypt(char *plaintext, char *key){
    for(int i=0; plaintext[i] != '\n'; i++){
        int currChar = plaintext[i];
        int keyChar = key[i];
        //Convert to number
        currChar = currChar - 65;
        keyChar = keyChar - 65;
        //Check for space
        if(currChar < 0) currChar = 26;
        if(keyChar < 0) keyChar = 26;

        //Encrypt
        int cipherChar = currChar + keyChar;
        cipherChar = cipherChar % 27;

        //Convert back to char
        //Check for space
        if(cipherChar == 26) cipherChar = 32;
        else cipherChar += 65;

        //Copy over to plaintext
        plaintext[i] = (char)cipherChar;
    }
    return plaintext;
}

//Send message to client
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

//Receive message from client
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
        if(strchr(buffer, term_char) != NULL) break;
    }
    return;
}

//Child process driver function
void child_process(int connection_socket){
    char message[70500];
    memset(message, '\0', 70500);
    //Verify connection
    recv_message(connection_socket, message);
    if(strcmp(message, CLIENT_NAME) != 0){
        fprintf(stderr, "SERVER: Not acceptable client, terminating");
        close(connection_socket);
        exit(2);
    }
    //Send identity to client
    memset(message, '\0', 70500);
    strcpy(message, NAME);
    send_message(connection_socket, message);

    //Receive plaintext
    memset(message, '\0', 70500);
    recv_message(connection_socket, message);
    //Copy to new variable
    char *plaintext = calloc(strlen(message)+1, sizeof(char));
    memset(plaintext, '\0', strlen(message)+1);
    strcpy(plaintext, message);

    //Receive key
    memset(message, '\0', 70500);
    recv_message(connection_socket, message);
    //Copy to new variable
    char *keytext = calloc(strlen(message)+1, sizeof(char));
    memset(keytext, '\0', strlen(message)+1);
    strcpy(keytext, message);

    //Check for bad input
    if(strlen(plaintext) > strlen(keytext)){
        fprintf(stderr, "SERVER: Bad input, message is larger than key");
        free(plaintext);
        free(keytext);
        exit(2);
    }

    char *ciphertext = encrypt(plaintext, keytext);

    //Send back ciphertext
    memset(message, '\0', 70500);
    strcpy(message, ciphertext);
    send_message(connection_socket, message);

    free(plaintext);
    free(keytext);
    exit(0);
}


// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
    int listen_port = atoi(argv[1]);
    int connection_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sizeof_clientInfo = sizeof(client_addr);

    //Create socket that will listen for connections
    int listen_socket = socket(AF_INET, SOCK_STREAM,0);
    if(listen_socket < 0){
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }

    //Set up address struct for server socket
    setupAddressStruct(&server_addr, listen_port);

    //Bind socket to port
    if(bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "ERROR binding port\n");
        exit(1);
    }

    while(1){
        //listen for connections
        listen(listen_socket, 5);
        //Accept connection request and create connection socket
        connection_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &sizeof_clientInfo);
        if(connection_socket < 0){
            fprintf(stderr, "ERROR on accept\n");
        }

        //Create new process
        pid_t spawnPid = fork();
        switch(spawnPid){
            case -1:
                fprintf(stderr, "SERVER: fork error\n");
                exit(1);
                break;
            case 0:
                child_process(connection_socket);
                //Some error occured
                fprintf(stderr, "SERVER: Error with child process\n");
                exit(1);
            default:
                break;
        }
    }
    return 0;
}