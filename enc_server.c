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

//Read from connection socket


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
    int connection_socket, charsRead;
    char buffer[256];
    memset(buffer, '\0', 256);
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

    for(int i=0; i < 5; i++){
        //listen for connections
        listen(listen_socket, 5);

        //Accept connection request and create connection socket
        connection_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &sizeof_clientInfo);
        if(connection_socket < 0){
            fprintf(stderr, "ERROR on accept\n");
        }
        printf("SERVER: Connected to client running at host %d port %d\n", 
                ntohs(client_addr.sin_addr.s_addr),
                ntohs(client_addr.sin_port));
        
        //Read from socket
        charsRead = recv(connection_socket, buffer, 255, 0);
        if (charsRead < 0){
            fprintf(stderr, "ERROR reading from socket");
        }
        printf("SERVER: I received this from the client: \"%s\"\n", buffer);

        charsRead = send(connection_socket, 
                    "I am the server, and I got your message", 39, 0); 
        if (charsRead < 0){
          fprintf(stderr, "ERROR writing to socket");
        }
        // Close the connection socket for this client
        close(connection_socket);
    }



    return 0;
}