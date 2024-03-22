#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <sys/select.h>
#include <time.h> // For time-out function with clock()

// Source Imports
#include "core/filemanip.h"

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 8888

#define TIMEOUT_TICKS 5 //In Seconds

void killclient(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void handlePrint(struct pdu* pdu) {
    if (pdu->type == 'E') {
        // Error message handling
        printf("[UDP-Client-Error]: %s\n", pdu->data);

    }    else if (pdu->type == 'D') {
        unsigned short seq_num = (unsigned char)pdu->data[0] << 8 | (unsigned char)pdu->data[1];

        char dat[98];
        memcpy(dat, pdu->data + 2, 98);

        printf("\n[UDP-Client] : SQ: %u, %c Data (char): \n%s\n", seq_num, pdu->type,dat);
    } 
    else  {
        // General message handling with a prefix
        printf("[UDP-Client]: Server reply: %c %s\n", pdu->type, pdu->data);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    int sock, addrLen = sizeof(serverAddr);
    struct pdu sentPDU, receivedPDU;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <host_ip> [port]\n", argv[0]);
        exit(1);
    }

    int port = (argc == 3) ? atoi(argv[2]) : PORT;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        killclient("socket creation failed");
    }

    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_aton(argv[1], &serverAddr.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // PDU List store:
    struct pdu* incoming_pdu_list = NULL;
    int i_count = 0;

    while (1)
    {
        printf("Enter message: ");
        if (fgets(sentPDU.data, sizeof(sentPDU.data), stdin) == NULL)
        {
            break; // Exit loop on input error or EOF
        }

        // Set the PDU type
        sentPDU.type = 'C';

        // Remove possible newline character
        sentPDU.data[strcspn(sentPDU.data, "\n")] = 0;

        // Send the message
        if (sendto(sock, &sentPDU, sizeof(sentPDU), 0, (struct sockaddr *)&serverAddr, addrLen) == -1)
        {
            killclient("sendto() failed");
        }

        struct timeval time_value;
        fd_set readfds;
        int select_sockevent_return; // Used to store the return value of select()

        clock_t lastTime = clock(), currentTime;

        do
        {
            FD_ZERO(&readfds);      // Clear the set
            FD_SET(sock, &readfds); // Add our descriptor to the set (needed for select)

            time_value.tv_sec = TIMEOUT_TICKS; // Set timeout duration in secs

            // Wait for an event on the socket, with a timeout: time_value
            select_sockevent_return = select(sock + 1, &readfds, NULL, NULL, &time_value);

            if (select_sockevent_return == -1) //Failed
            {
                killclient("select() failed");
            }
            else if (select_sockevent_return) // Data is available, this won't block
            {
                memset(&receivedPDU, 0, sizeof(receivedPDU)); // Clear the buffer
                if (recvfrom(sock, &receivedPDU, sizeof(receivedPDU), 0, (struct sockaddr *)&serverAddr, &addrLen) == -1)
                {
                    killclient("recvfrom() failed");
                }

            incoming_pdu_list = realloc(incoming_pdu_list, (i_count + 1) * sizeof(struct pdu));
            if (!incoming_pdu_list) {
                perror("Failed to allocate memory for PDUs");
                exit(2);
            }
            incoming_pdu_list[i_count] = receivedPDU;
            i_count++;

            handlePrint(&receivedPDU);

                lastTime = clock(); // Reset the timer
            }
            else //! TIMEOUT CONDITION \/ \/ \/
            {
                printf("No message received for %d seconds.\n", TIMEOUT_TICKS);
            }

            currentTime = clock();

        } while (receivedPDU.type != 'F' && receivedPDU.type != 'E');

        printf("DO WHILE ENDED\n");

for(int i = 0; i < i_count; i++) {
    // Extract the first two bytes
    unsigned char high_byte = incoming_pdu_list[i].data[0];
    unsigned char low_byte = incoming_pdu_list[i].data[1];

    // Convert to 16-bit value
    uint16_t seq_num = (high_byte << 8) | low_byte;

    // Print the sequence number
    printf("Sequence Number: %u\n", seq_num);
}

if (i_count > 0) {
    const char* output_filename = "output_file.bin";
    int result = rebuild_file_from_pdus(output_filename, incoming_pdu_list, i_count);
    if (result != 0) {
        fprintf(stderr, "Failed to rebuild file from PDUs\n");
    } else {
        printf("File successfully rebuilt from PDUs\n");
    }
}
    }



    close(sock);
    return 0;
}
