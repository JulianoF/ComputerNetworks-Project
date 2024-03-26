#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 512 // Max length of buffer
#define PORT 8888  // The port on which to listen for incoming data

// In-Memory File Requests Object Max Size:
#define MAX_REQUESTS 10

// Source code imports
#include "core/filemanip.h"

void killserver(char *s)
{
    perror(s);
    exit(1);
}

//* File Requested by Client, will be stored in-memory, so that if any errors occur we can refer to this object for re-transmission

struct file_request
{

    // Client
    uint16_t c_port;
    char *c_addr;

    // File
    char filename[DATA_LEN];
    uint32_t pdu_count;
    struct pdu *pdu_list;
};

struct file_request *requests[MAX_REQUESTS] = {0}; // init all to null

struct file_request *handle_file_request(struct pdu received_pdu, char *c_a, uint16_t c_p)
{
    // Look for an empty spot in the requests array
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        if (requests[i] == NULL)
        {
            // Allocate memory for a new request
            struct file_request *new_request = (struct file_request *)malloc(sizeof(struct file_request));
            if (new_request == NULL)
            {
                perror("Failed to allocate memory for new request");
                return NULL; // Failure
            }

            // Set the client's address and port
            new_request->c_addr = strdup(c_a); // String Dupe to ensure it persists
            new_request->c_port = c_p;

            // Copy the filename from the received PDU
            strncpy(new_request->filename, received_pdu.data, DATA_LEN);

            // Initialize other fields
            new_request->pdu_count = 0;
            new_request->pdu_list = load_file_into_pdus(received_pdu.data, &new_request->pdu_count);

            if (new_request->pdu_list != NULL)
            {
                printf("\n OK: Successful read of %s", new_request->filename);
            } else {
                return NULL;
            }

            // Save the new request in the first empty spot found
            requests[i] = new_request;
            printf("\n>> Saved Incoming Type `C` Request ! \n");

            return new_request; // Success
        }
    }

    return NULL; // Array is full, or another error occurred
}


struct file_request *lookup_file_request(char *c_a, uint16_t c_p)
{
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        if (requests[i] != NULL)
        {
            // Check if the client address and port match
            if (strcmp(requests[i]->c_addr, c_a) == 0 && requests[i]->c_port == c_p)
            {
                return requests[i]; // Matching request found
            }
        }
    }
    return NULL; // No matching request found
}

void delete_file_request(char *c_a, uint16_t c_p) {
    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (requests[i] != NULL) {
            // Check if the client address and port match
            if (strcmp(requests[i]->c_addr, c_a) == 0 && requests[i]->c_port == c_p) {
                // Free client address string
                free(requests[i]->c_addr);
                requests[i]->c_addr = NULL;

                free(requests[i]->pdu_list); //free pdu list built
                requests[i]->pdu_list = NULL;

                // Finally, free the request object itself and remove it from the requests array
                free(requests[i]);
                requests[i] = NULL; // Mark as available

                printf("\n>> Deleted file request from %s:%d successfully.\n", c_a, c_p);
                return; // Exit after deletion
            }
        }
    }
    printf("\n>> No matching file request found for %s:%d.\n", c_a, c_p);
}

int main(void)
{

    struct sockaddr_in self_socket_ADDR, incoming_socket_ADDR;

    int s, alen = sizeof(incoming_socket_ADDR), recv_len;
    char buf[BUFLEN];

    // Create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        killserver("socket");
    }

    // Zero out the structure
    memset((char *)&self_socket_ADDR, 0, sizeof(self_socket_ADDR));

    self_socket_ADDR.sin_family = AF_INET;   // ipv4
    self_socket_ADDR.sin_port = htons(PORT); // set port
    self_socket_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port
    if (bind(s, (struct sockaddr *)&self_socket_ADDR, sizeof(self_socket_ADDR)) == -1)
    {
        killserver("bind");
    }

    // Keep listening for data
    while (1)
    {
        printf("Waiting for data...");
        fflush(stdout);

        // Try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&incoming_socket_ADDR, &alen)) == -1)
        {
            killserver("recvfrom()");
        }

        //* 1. --------------- Decoding Incoming PDU ---

        struct pdu received_pdu;
        received_pdu.type = buf[0];                       // Type of PDU
        memcpy(received_pdu.data, buf + 1, recv_len - 1); // Assuming pure 100 bytes Data (Only Client receives complete PDU packets)

        //* 2. --------------- Performing Requested Action(s) ---

        if (received_pdu.type == 'C')
        {
            char *c_a = inet_ntoa(incoming_socket_ADDR.sin_addr);
            uint16_t c_p = ntohs(incoming_socket_ADDR.sin_port);

            struct file_request *req = handle_file_request(received_pdu, c_a, c_p);

            //Make sure file_request is successful:

            if(req == NULL) {

                struct pdu ErrorPDU;
                ErrorPDU.type = 'E';
                    strncpy(ErrorPDU.data, "Couldn't send file, it either isn't available or doesn't exist", PDU_DATA_SIZE - 1);
                    ErrorPDU.data[PDU_DATA_SIZE - 1] = '\0'; // Ensure null-termination

                if (sendto(s, &ErrorPDU, sizeof(ErrorPDU), 0, (struct sockaddr *)&incoming_socket_ADDR, alen) == -1)
                    {
                        perror("sendto()");
                        // Handle error, maybe break out of the loop or attempt to resend
                    }
                continue;
            }

            if (req != NULL && req->pdu_list != NULL)
            {
                printf("PDU C: %d\n",req->pdu_count);

                //Sending Length Packet
                struct pdu lenPDU;
                lenPDU.type = 'O';
                set_pdu_seq_num(&lenPDU, req->pdu_count);

                //Send Ok Length Packet:
                if (sendto(s, &lenPDU, sizeof(&lenPDU), 0, (struct sockaddr *)&incoming_socket_ADDR, alen) == -1)
                    {
                        perror("sendto()");
                        // Handle error, maybe break out of the loop or attempt to resend
                    }
                
                
                for (uint32_t i = 0; i < req->pdu_count; i++) //i is 32bit unsigned int to match pdu seq. num
                {

                    struct pdu *sending_pdu = &req->pdu_list[i];
                    int pdu_length = sizeof(*sending_pdu);

                    uint32_t seq_num = get_pdu_seq_num(sending_pdu);

                    printf("\n[UDP-Serv] : SQ: %u, %c\n", seq_num, sending_pdu->type);

                    if (sendto(s, sending_pdu, pdu_length, 0, (struct sockaddr *)&incoming_socket_ADDR, alen) == -1)
                    {
                        perror("sendto()");
                        // Handle error, maybe break out of the loop or attempt to resend
                    }

                    usleep(15);

                }
                //DEBUG:
                rebuild_file_from_pdus("quality_Z.mp4",req->pdu_list, req->pdu_count);
            }
        }
        if (received_pdu.type == 'E')
        {

            char *c_a = inet_ntoa(incoming_socket_ADDR.sin_addr);
            uint16_t c_p = ntohs(incoming_socket_ADDR.sin_port);

            struct file_request *req = lookup_file_request(c_a, c_p); //Either the User's OG file, or NULL

            if(req == NULL){
                printf("Failed to find FQ");
                continue;
            }

            if(req != NULL){

                uint32_t seq_num_in_need = get_pdu_seq_num(&received_pdu);
                printf("\nERROR: Client needs PDU with SQ: %d",seq_num_in_need);

                struct pdu *sending_pdu = &req->pdu_list[seq_num_in_need];
                int pdu_length = sizeof(*sending_pdu);

                if (sendto(s, sending_pdu, pdu_length, 0, (struct sockaddr *)&incoming_socket_ADDR, alen) == -1)
                    {
                        perror("sendto()");
                        // Handle error, maybe break out of the loop or attempt to resend
                    }
            }

        }

        if (received_pdu.type == 'O')
        {
            char *c_a = inet_ntoa(incoming_socket_ADDR.sin_addr);
            uint16_t c_p = ntohs(incoming_socket_ADDR.sin_port);
            printf("HANDLING OK....\n");

            delete_file_request(c_a,c_p);
        }

        //* 3. --------------- Debug Printing ---

        printf("Received packet from %s:%d\n", inet_ntoa(incoming_socket_ADDR.sin_addr), ntohs(incoming_socket_ADDR.sin_port));
        printf("PDU Type: %c\n", received_pdu.type);

        printf("PDU Data: %.*s\n", recv_len - 1, received_pdu.data);

        // Now reply to the client with the same data
        if (sendto(s, &received_pdu, sizeof(received_pdu), 0, (struct sockaddr *)&incoming_socket_ADDR, alen) == -1)
        {
            killserver("sendto()");
        }
    }

    close(s);
    return 0;
}
