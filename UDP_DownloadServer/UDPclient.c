#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Source Imports
#include "core/filemanip.h"

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 8888

void killclient(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddr;
    int sock, addrLen = sizeof(serverAddr);
    struct pdu sentPDU, receivedPDU;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host_ip> [port]\n", argv[0]);
        exit(1);
    }

    int port = (argc == 3) ? atoi(argv[2]) : PORT;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        killclient("socket creation failed");
    }

    memset((char *) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_aton(argv[1], &serverAddr.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    while(1) {
        printf("Enter message: ");
        if (fgets(sentPDU.data, sizeof(sentPDU.data), stdin) == NULL) {
            break; // Exit loop on input error or EOF
        }

        // Set the PDU type
        sentPDU.type = '1'; // Example type, adjust as needed

        // Remove possible newline character
        sentPDU.data[strcspn(sentPDU.data, "\n")] = 0;

        // Send the message
        if (sendto(sock, &sentPDU, sizeof(sentPDU), 0, (struct sockaddr *) &serverAddr, addrLen) == -1) {
            killclient("sendto() failed");
        }

        // Receive a reply and print it
        memset(&receivedPDU, 0, sizeof(receivedPDU)); // Clear the buffer
        if (recvfrom(sock, &receivedPDU, sizeof(receivedPDU), 0, (struct sockaddr *) &serverAddr, &addrLen) == -1) {
            killclient("recvfrom() failed");
        }
        printf("Server reply: %c %s\n", receivedPDU.type, receivedPDU.data);
    }

    close(sock);
    return 0;
}
