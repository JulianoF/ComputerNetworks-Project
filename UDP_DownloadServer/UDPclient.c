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

//! -------------------------- Test Code -----------------------
        int pdu_count = 0;
        struct pdu* pdu_list = load_file_into_pdus("./example.dat", &pdu_count);

        if (pdu_list == NULL) {
            fprintf(stderr, "Failed to load PDUs. Exiting.\n");
            return EXIT_FAILURE;
        }

        printf("** PDU Count: %d ***\n", pdu_count);

        //Print 10 or less pdus ()
        for (int i = 0; i < pdu_count && i < 10; i++) {
            unsigned short seq_num = (unsigned char)pdu_list[i].data[0] << 8 | (unsigned char)pdu_list[i].data[1];

        char dat[98]; // No need for initialization here since we'll be copying into it
        memcpy(dat, pdu_list[i].data + 2, 98);

            printf("\n Parsed -> PDU %d: SeqNum = %u, Type = %c DAT: %s\n", i, seq_num, pdu_list[i].type,dat);
        }



        rebuild_file_from_pdus("example2.dat", pdu_list, pdu_count);

        free(pdu_list);


//! -------------------------------------------------------------

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
