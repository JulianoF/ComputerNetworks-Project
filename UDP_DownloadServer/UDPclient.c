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
#define PORT 8888

#define TIMEOUT_TICKS 5 // In Seconds

struct sequence_jump
{
    uint32_t cur;
    uint32_t last;
};

void killclient(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void handlePrint(struct pdu *pdu)
{
    if (pdu->type == 'E')
    {
        // Error message handling
        printf("[UDP-Client-Error]: %s\n", pdu->data);
    }
    else if (pdu->type == 'D')
    {
        uint32_t seq_num = get_pdu_seq_num(pdu);

        char dat[97];
        memcpy(dat, pdu->data + 3, 97);

        printf("\n[UDP-Client] : SQ: %u, %c Data (char): \n%s\n", seq_num, pdu->type, dat);
    }
    else
    {
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
    struct pdu *incoming_pdu_list = NULL;

    while (1)
    {
        printf("Enter message: ");
        if (fgets(sentPDU.data, sizeof(sentPDU.data), stdin) == NULL)
        {
            break; // Exit loop on input error or EOF
        }

        sentPDU.type = 'C'; // msg req

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

        uint32_t i_pdu_count = 0;
        uint32_t Correct_PDU_count = 0; // This will be the Number of PDU packets the message contains (sent via the 'O' Response)

        uint32_t last_seq_num_sent = 0; // This is for checking +1 continiity

        struct sequence_jump *mistake = NULL;
        size_t mistake_count = 0;

        char modified_filename[256];
        extract_filename(sentPDU.data, modified_filename, sizeof(modified_filename)); // Extract Filename out of MY request

        do
        {
            FD_ZERO(&readfds);      // Clear the set
            FD_SET(sock, &readfds); // Add our descriptor to the set (needed for select)

            time_value.tv_sec = TIMEOUT_TICKS; // Set timeout duration in secs

            // Wait for an event on the socket, with a timeout: time_value
            select_sockevent_return = select(sock + 1, &readfds, NULL, NULL, &time_value);

            if (select_sockevent_return == -1) // Failed
            {
                printf("Oops something went wrong please, try again or restart client application...");
                break;
            }
            else if (select_sockevent_return) // Data is available, this won't block
            {
                memset(&receivedPDU, 0, sizeof(receivedPDU)); // Clear the buffer
                if (recvfrom(sock, &receivedPDU, sizeof(receivedPDU), 0, (struct sockaddr *)&serverAddr, &addrLen) == -1)
                {
                    killclient("recvfrom() failed");
                }

                //* Check what kind of Received PDU it is (Specifically it's an O)
                if (receivedPDU.type == 'O')
                {
                    Correct_PDU_count = get_pdu_seq_num(&receivedPDU);
                }
                else if (receivedPDU.type == "E")
                {
                    break;
                }
                else
                {
                    uint32_t seq_n = get_pdu_seq_num(&receivedPDU);

                    uint32_t SEQ_DIFF = seq_n - last_seq_num_sent; // cur-last

                    if (last_seq_num_sent == 0 && seq_n == 0)
                    { // First case:
                        printf("SQ_--");
                    }
                    else if (SEQ_DIFF > 1 || 1 < SEQ_DIFF)
                    { // Adjusted for clarity
                        // printf("Jump_in_SQ_--\n");

                        // Resize the jumps array to accommodate one more sequence_jump
                        struct sequence_jump *temp = realloc(mistake, (mistake_count + 1) * sizeof(struct sequence_jump));
                        if (temp == NULL)
                        {
                            // Handle realloc failure
                            perror("Failed to resize jumps array");
                            free(mistake); // It's a good practice to free allocated memory if you're going to exit due to an error
                            exit(EXIT_FAILURE);
                        }
                        mistake = temp;

                        // Add the new jump information
                        mistake[mistake_count].cur = seq_n;
                        mistake[mistake_count].last = last_seq_num_sent;
                        mistake_count++;
                    }

                    last_seq_num_sent = seq_n; // reset last

                    incoming_pdu_list = realloc(incoming_pdu_list, (i_pdu_count + 1) * sizeof(struct pdu));
                    if (!incoming_pdu_list)
                    {
                        perror("Failed to allocate memory for PDUs");
                        exit(2);
                    }
                    incoming_pdu_list[i_pdu_count] = receivedPDU;
                    i_pdu_count++;
                }

                handlePrint(&receivedPDU);

                lastTime = clock(); // Reset the timer
            }
            else //! TIMEOUT CONDITION \/ \/ \/
            {
                printf("No message received for %d seconds. MAX S %d\n", TIMEOUT_TICKS, Correct_PDU_count);
            }

            currentTime = clock();

        } while (receivedPDU.type != 'F' && receivedPDU.type != 'E');

        printf("EXITED DO WHILE ON CLIENT");

        //* ----------------- After While Loop of Receiving Packets (filled incoming_pdu_list buffer) -----------

        // struct pdu *cleaned_pdu = validate_pdu_list(incoming_pdu_list, i_pdu_count);

        if (mistake_count == 0)
        {
            printf("FILE ARRIVED WITH NO MISTAKES\n");
        }

        for (size_t j = 0; j < mistake_count; j++)
        {
            uint32_t start = mistake[j].last + 1;
            uint32_t end = mistake[j].cur - 1;

            for (uint32_t missing_seq = start; missing_seq <= end; missing_seq++)
            {
                struct pdu errorPDU;
                memset(&errorPDU, 0, sizeof(errorPDU));
                errorPDU.type = 'E';                     // Error PDU type
                set_pdu_seq_num(&errorPDU, missing_seq); // Set missing sequence number

                // Send the error PDU to request missing sequence number
                if (sendto(sock, &errorPDU, sizeof(errorPDU), 0, (struct sockaddr *)&serverAddr, addrLen) == -1)
                {
                    killclient("sendto() failed on error PDU");
                }

                // Wait for the server to send the requested PDU
                memset(&receivedPDU, 0, sizeof(receivedPDU)); // Clear the buffer
                if (recvfrom(sock, &receivedPDU, sizeof(receivedPDU), 0, (struct sockaddr *)&serverAddr, &addrLen) == -1)
                {
                    killclient("recvfrom() failed on requested PDU");
                }

                // handlePrint(&receivedPDU);

                // Add the received PDU to incoming_pdu_list
                incoming_pdu_list = realloc(incoming_pdu_list, (i_pdu_count + 1) * sizeof(struct pdu));
                if (!incoming_pdu_list)
                {
                    perror("Failed to allocate memory for incoming PDUs");
                    exit(EXIT_FAILURE);
                }
                incoming_pdu_list[i_pdu_count] = receivedPDU;
                i_pdu_count++;
            }
        }

        //* Send OK to Server now:
        struct pdu okPDU;
        memset(&okPDU, 0, sizeof(okPDU));
        okPDU.type = 'O';
        printf("SENT OK");
        // Send the error PDU to request missing sequence number
        if (sendto(sock, &okPDU, sizeof(okPDU), 0, (struct sockaddr *)&serverAddr, addrLen) == -1)
        {
            killclient("sendto() failed to ok ");
        }

        if (mistake_count > 0)
        {
            printf("File Needs to be Re-sorted as Mistakes were made & corrected.\n");

            size_t update_threshold = i_pdu_count / 20; // Update every 5% of the total count
            size_t update_next = update_threshold;      // When to show the next update

            for (size_t i = 0; i < i_pdu_count - 1; i++)
            {
                int swapped = 0; // Use 0 as false
                for (size_t j = 0; j < i_pdu_count - i - 1; j++)
                {
                    uint32_t seq_num_j = get_pdu_seq_num(&incoming_pdu_list[j]);
                    uint32_t seq_num_next = get_pdu_seq_num(&incoming_pdu_list[j + 1]);

                    // Swap if the sequence number of the current PDU is greater than the next PDU
                    if (seq_num_j > seq_num_next)
                    {
                        struct pdu temp = incoming_pdu_list[j];
                        incoming_pdu_list[j] = incoming_pdu_list[j + 1];
                        incoming_pdu_list[j + 1] = temp;
                        swapped = 1; // Use 1 as true
                    }
                }

                // If no two elements were swapped by the inner loop, then break
                if (!swapped)
                {
                    break;
                }

                // Show progress update
                if (i >= update_next)
                {
                    printf("Sorting Progress: Approximately %zu%% complete...\n", (i + 1) * 100 / (i_pdu_count - 1));
                    update_next += update_threshold;
                }
            }

            printf("Sorting Complete.\n");
        }
        if (i_pdu_count > 0)
        {
            printf("FNAME: %s", modified_filename);
            int result = rebuild_file_from_pdus(modified_filename, incoming_pdu_list, i_pdu_count);
            if (result != 0)
            {
                fprintf(stderr, "Failed to rebuild file from PDUs\n");
            }
            else
            {
                printf("File successfully rebuilt from PDUs\n");
            }
        }
    }

    close(sock);
    return 0;
}
