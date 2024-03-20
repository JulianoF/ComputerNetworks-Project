#include "ConnectionLib.h"

struct sockaddr_in server, client;

int initServer(int port, char* ip){
    int sd;
     printf("Opening TCP Server on Port %d\n",port);
      
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }else{
        printf("Socket successfully created\n");
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    

    if(bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("Can't bind name to socket\n");
        return -1;
    }else{
        printf("Socket successfully binded\n");
    }

    if((listen(sd, 5)) != 0) { 
        perror("Listen failed\n"); 
        return -1;
    }else{
        printf("Server listening...\n");
    }
    return sd;
}

void acceptClient(int sd){

}

void closeServer(int sd){
    close(sd);
}