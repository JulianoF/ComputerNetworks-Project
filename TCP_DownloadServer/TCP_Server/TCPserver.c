#define SERVER_TCP_PORT 8000
#define LOOPBACK_IP "127.0.0.1"

#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#include "ConnectionLib.h"

int main(int argc, char** argv){
    char* ip;
    int port, sd;
    
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

    //printf("PORT: %d  IP: %s \n",port,ip);

    sd = initServer(port,ip);
    if(sd < 0){
        printf("Server Failure Shutting down\n");
        closeServer(sd);
        exit(1);
    }

    closeServer(sd);
    return 0;    
}