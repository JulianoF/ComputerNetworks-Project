#define SERVER_TCP_PORT 8000
#define BUFLEN 256
#define IP "127.0.0.1" 

#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char** argv){
    int sd, port;
    struct sockaddr_in server;
    char buffer[BUFLEN];
    
    if(argc == 2){
        port = atoi(argv[1]);
        if(port == 0){
            port = SERVER_TCP_PORT;
        }
    }else{
        port = SERVER_TCP_PORT;
    }

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }else{
        printf("Socket successfully created\n");
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(IP);

    if (connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connection with the server failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Connected to the server\n");
    }

    printf("Waiting for Data..\n");
    while(1){
        ssize_t status = read(sd, buffer, sizeof(buffer));
        if(status <= 0){
            break;
        }
        printf("Message from Server: %s \n", buffer);
    }
    close(sd);
    return 0;
}