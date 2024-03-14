#define SERVER_TCP_PORT 8000
#define BUFLEN 256 
#define MESSAGE "Hello"

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;
    char buffer[BUFLEN];
    
    if(argc == 2){
        port = atoi(argv[1]);
        if(port == 0){
            port = SERVER_TCP_PORT;
        }
    }else{
        port = SERVER_TCP_PORT;
    }
    printf("Opening TCP Server on Port %d\n",port);
      
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }else{
        printf("Socket successfully created\n");
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    

    if(bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("Can't bind name to socket\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Socket successfully binded\n");
    }

    if((listen(sd, 5)) != 0) { 
        perror("Listen failed\n"); 
        exit(EXIT_FAILURE); 
    }else{
        printf("Server listening...\n");
    }

    while(1) {
        ssize_t bytes_written = 0;
        client_len = sizeof(client);
        new_sd = accept(sd, (struct sockaddr *)&client, &client_len);

        if(new_sd < 0){
            perror("Can't accept client \n");
            exit(EXIT_FAILURE);
        }
        bytes_written = (new_sd, MESSAGE, strlen(MESSAGE),0);
        if(bytes_written >= 20){
            close(new_sd);
            break;
        }
    }

    close(sd);

}