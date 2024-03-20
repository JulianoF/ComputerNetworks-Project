#define SERVER_TCP_PORT 8000
#define LOOPBACK_IP "127.0.0.1"

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
    char buffer[256];
    char* ip;
    int port, sd, result;
    struct sockaddr_in server;
    
    switch(argc){
        case 2:
            port = atoi(argv[1]);
            if(port == 0){
                port = SERVER_TCP_PORT;
            }
            ip = LOOPBACK_IP;
            break;
        case 3:
            port = atoi(argv[1]);
            if(port == 0){
                port = SERVER_TCP_PORT;
            }
            ip = argv[2];
            break;
        default:
            port = SERVER_TCP_PORT;
            ip = LOOPBACK_IP;
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
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connection with the server failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Connected to the server\n");
    }

    while(1){
        printf("What filename are you requesting?: ");
        memset(buffer,0,sizeof(buffer));
        scanf("%s", buffer);

        result = write(sd,buffer,strlen(buffer));

        if (result < 0){
            perror("ERROR while writing to socket");
            exit(1);
        }
    }
    close(sd);

    return 0;
}
