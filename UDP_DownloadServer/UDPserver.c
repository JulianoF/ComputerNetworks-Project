#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 512  // Max length of buffer
#define PORT 8888   // The port on which to listen for incoming data

// Source code imports
#include "core/filemanip.h"

void killserver(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in self_socket_ADDR, incoming_socket_ADDR;
    
    int s, alen = sizeof(incoming_socket_ADDR) , recv_len;
    char buf[BUFLEN];
    
    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        killserver("socket");
    }
    
    // Zero out the structure
    memset((char *) &self_socket_ADDR, 0, sizeof(self_socket_ADDR));
    
    self_socket_ADDR.sin_family = AF_INET; //ipv4
    self_socket_ADDR.sin_port = htons(PORT); //set port
    self_socket_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind socket to port
    if(bind(s, (struct sockaddr*)&self_socket_ADDR, sizeof(self_socket_ADDR)) == -1) {
        killserver("bind");
    }
    
    // Keep listening for data
    while(1) {
        printf("Waiting for data...");
        fflush(stdout);
        
        // Try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &incoming_socket_ADDR, &alen)) == -1) {
            killserver("recvfrom()");
        }
        
        struct pdu received_pdu;
        received_pdu.type = buf[0]; // First byte is type
        memcpy(received_pdu.data, buf + 1, recv_len - 1); // The rest is data
        
        // Print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(incoming_socket_ADDR.sin_addr), ntohs(incoming_socket_ADDR.sin_port));
        printf("PDU Type: %c\n", received_pdu.type);
        printf("PDU Data: %.*s\n", recv_len - 1, received_pdu.data);
        
        // Now reply to the client with the same data
        if (sendto(s, &received_pdu, sizeof(received_pdu), 0, (struct sockaddr*) &incoming_socket_ADDR, alen) == -1) {
            killserver("sendto()");
        }
    }

    close(s);
    return 0;
}
