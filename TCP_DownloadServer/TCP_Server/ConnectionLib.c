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

int acceptClient(int sd){
    int client_sd, client_len;
    client_len = sizeof(client);

    client_sd = accept(sd, (struct sockaddr *)&client, &client_len);
    if(client_sd < 0){
        perror("Can't accept client \n");
        return -1;
    }
    printf("Client connected from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    while(1){
        printf("Waiting for Filename From Client\n");
        char *fromUser = handleFilename(client_sd);
        if (fromUser != NULL) {
            printf("Processing filename: %s\n", fromUser);
            
        } else {
            printf("Failed to receive filename.\n");
            break;
            // Handle the error accordingly
        }
    }
    

    close(client_sd);
    return client_sd; 
}

char* handleFilename(int client_sd){
    char* filename; 
    filename = malloc(sizeof(char) * 256);
    if (filename == NULL) {
        perror("malloc");
        return NULL;
    }
    int recv_bytes = recv(client_sd, filename, sizeof(filename)*256, 0);
    if (recv_bytes < 0) {
        perror("recv");
        free(filename);
        return NULL;
    }else if(recv_bytes == 0){
        printf("Client disconnected\n");
        free(filename);
        return NULL;
    }
    
    filename[recv_bytes] = '\0';
    printf("Received Filename: %s\n", filename);

    return filename;
}



void closeServer(int sd){
    close(sd);
}

void closeClient(int client_sd){
    close(client_sd);
}