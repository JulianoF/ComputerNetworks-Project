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
        if(fromUser == NULL){
            printf("Connection Closed. Failed to receive filename.\n");
            break;
        }
        else if(strcmp(fromUser,"quit")==0){
            break;
            
        }
        else if (fromUser != NULL) {
            printf("Processing filename: %s\n", fromUser);
            int stat = sendFile(client_sd,fromUser);
            if(stat < 0){
                continue;
            }else{
                break;
            }
            
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
    return filename;
}

int sendFile(int client_sd, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        char* errorMsg = "!# File not found Error From Server";
        send(client_sd, errorMsg, strlen(errorMsg), 0);
        return -1;
    } else {
        char buffer[MAX_PAYLOAD_SIZE];
        size_t bytesRead;
        size_t bytesSent= 0;
        while ((bytesRead = fread(buffer, 1, MAX_PAYLOAD_SIZE, file)) > 0) {
            sleep(1);
            ssize_t sent = send(client_sd, buffer, bytesRead, 0);
            bytesSent += sent;
            printf("Sending %zu bytes\n",sent);
        }
        printf("File Sent to Client totaling: %zu Bytes!\n",bytesSent);
        fclose(file);
        return 0;
    }
}

void closeServer(int sd){
    close(sd);
}

void closeClient(int client_sd){
    close(client_sd);
}