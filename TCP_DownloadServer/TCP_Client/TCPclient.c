#define SERVER_TCP_PORT 8000
#define LOOPBACK_IP "127.0.0.1"

#define MAX_PAYLOAD_SIZE 100 

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

void receive_file(int sd, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL){
        perror("Error opening file for writing");
        return;
    }

    char buffer[MAX_PAYLOAD_SIZE];
    ssize_t bytesRead;
    size_t bytesTotal= 0;
    while ((bytesRead = recv(sd, buffer, MAX_PAYLOAD_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
        printf("Recieved %zu Bytes\n",bytesRead);
        bytesTotal += bytesRead;
    }
    if (bytesRead < 0) {
        perror("Error receiving file data");
    }
    printf("Total Received: %zu Bytes\n",bytesTotal);
    fclose(file);
}

int main(int argc, char** argv){
    char filename[256];
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
        perror("Connection with the server failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Connected to the server\n");
    }

    while(1){
        printf("What filename are you requesting?: ");
        memset(filename,0,sizeof(filename));
        scanf("%s", filename);

        result = write(sd,filename,strlen(filename));

        if (result < 0){
            perror("ERROR while writing to socket");
            exit(1);
        }
        if(strcmp(filename,"quit")==0){
            break;
        }
        char buffer[MAX_PAYLOAD_SIZE];
        ssize_t bytesRead = recv(sd, buffer, MAX_PAYLOAD_SIZE, 0);

        if (bytesRead > 0) {
            if (buffer[0] != '!' && buffer[1] != '#') {
                printf("File transfer started...\n");
                receive_file(sd, filename);
                printf("File received successfully.\n");
                break;
            } else {
                printf("Error: %s\n", buffer);
            }
        } else {
            printf("Error receiving data from server.\n");
        }

    }

    close(sd);

    return 0;
}
