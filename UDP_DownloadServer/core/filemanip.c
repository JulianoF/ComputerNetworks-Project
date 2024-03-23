#include "filemanip.h"

//In C's `char` datatype, It's actually signed, so that's why max is 127 chars, and MSB being a sign when promoted to larger Value
// 0: 127 small chars, 1: 127 big chars, the issue here is when casting to int, It was taking that 1 in signed char as negative, and underflowing seq number

//* Causing Behaviors like this in the server:

//[UDP-Serv] : SQ: 127, D packet
//[UDP-Serv] : SQ: 4294967168, D packet

//!Notice: after a signed char hit's max, it totally overflows a 32 bit int

uint32_t get_pdu_seq_num(struct pdu *pdu) {

    //!IMPORTANT: Cast to unsigned char before shifting to ensure no sign extension and avoid overflow issues, read above
    uint32_t seq_num = ((uint32_t)(unsigned char)pdu->data[0] << 16) | 
                       ((uint32_t)(unsigned char)pdu->data[1] << 8) | 
                        (uint32_t)(unsigned char)pdu->data[2];
    return seq_num;
}

void set_pdu_seq_num(struct pdu *pdu, uint32_t seq_num) {
    // Clear the upper 8 bits to ensure only 24 bits are encoded
    seq_num &= 0xFFFFFF;

    // Direct assignment to char is fine here since we are only storing the lower 8 bits
    pdu->data[0] = (char)((seq_num >> 16) & 0xFF);
    pdu->data[1] = (char)((seq_num >> 8) & 0xFF);
    pdu->data[2] = (char)(seq_num & 0xFF);
}

//* ----------------------- File -> PDU List -----------------------

struct pdu *load_file_into_pdus(const char *filename, int *pdu_count)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    int total_data_PDUs = (file_size + DATA_LEN - 1) / DATA_LEN; // Calculate the total number of PDUs required
    struct pdu *pdus = (struct pdu *)malloc(total_data_PDUs * sizeof(struct pdu));

    if (!pdus)
    {
        fclose(file);
        return NULL;
        printf("No PDUs Allocated?");
    }

    uint32_t seq_num = 0; // Max Size: 4,294,967,295
    size_t bytes_read;
    char buffer[DATA_LEN];

    for (int i = 0; i < total_data_PDUs; i++)
    {
        bytes_read = fread(buffer, 1, DATA_LEN, file);

        pdus[i].type = (i == total_data_PDUs - 1) ? 'F' : 'D'; // Mark the last PDU if it is

        set_pdu_seq_num(&pdus[i], seq_num); //Sets 3 byte sequence number in chars

        if (bytes_read > 0)
        {
            memcpy(pdus[i].data + SEQ_LEN, buffer, bytes_read);
        }
        seq_num++;
    }

    *pdu_count = total_data_PDUs;
    fclose(file);
    return pdus;
}

//* ----------------------- PDU List -> File -----------------------

int rebuild_file_from_pdus(const char *output_filename, struct pdu *pdu_list, int pdu_count)
{
    if (pdu_list == NULL || pdu_count <= 0)
    {
        fprintf(stderr, "Invalid PDU list or count.\n");
        return 1; // error 1
    }

    FILE *file = fopen(output_filename, "wb");
    if (!file)
    {
        perror("Failed to open output file");
        return 2; // error for read fail
    }

    for (int i = 0; i < pdu_count; i++)
    {
        size_t data_length = (i == pdu_count - 1 && pdu_list[i].type == 'F') ? strlen(pdu_list[i].data + SEQ_LEN) : DATA_LEN;

        // Write the data part of the PDU to the file, skipping the sequence number
        if (fwrite(pdu_list[i].data + SEQ_LEN, 1, data_length, file) != data_length)
        {
            fprintf(stderr, "Failed to write data to output file.\n");
            fclose(file);
            return 3; // Error code write fail
        }

        // Stop if this is the final PDU
        if (pdu_list[i].type == 'F')
            break;
    }

    fclose(file);
    return 0; // Success
}

// ----------------------- PDU List pipelining to ensure correctness -----------------------

struct pdu *validate_pdu_list(struct pdu *dirty_pdu_list, int pdu_count)
{

    //* ( 1. ) Sorting PDU's based on their sequence numbers:

    struct pdu *sorted_pdu_list = (struct pdu *)malloc(pdu_count * sizeof(struct pdu));

    if (sorted_pdu_list == NULL)
    {
        // Handle memory allocation failure
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Copy dirty_pdu_list to sorted_pdu_list
    for (int i = 0; i < pdu_count; i++)
    {
        sorted_pdu_list[i] = dirty_pdu_list[i];
    }

    // Bubble sort algorithm with seq/num compares
    for (int i = 0; i < pdu_count - 1; i++)
    {
        for (int j = 0; j < pdu_count - i - 1; j++)
        {

            uint32_t seq_num_j = get_pdu_seq_num(&sorted_pdu_list[j]);
            uint32_t seq_num_j1 = get_pdu_seq_num(&sorted_pdu_list[j + 1]);

            // If the current item's sequence number is greater than the next, swap them
            if (seq_num_j > seq_num_j1)
            {
                struct pdu temp = sorted_pdu_list[j];
                sorted_pdu_list[j] = sorted_pdu_list[j + 1];
                sorted_pdu_list[j + 1] = temp;
            }
        }
    }

    //* ( 2. ) - Ensuring there are no gaps in sequence numbers: EX: 0,1,3,4 in this case 2 is missing
    //*        - Ensuring that Final PDU packet is included

    int valid_count = pdu_count; // Assume all PDUs are valid initially

    for (int i = 0; i < pdu_count - 1; i++)
    {
        uint32_t seq_num_current = get_pdu_seq_num(&sorted_pdu_list[i]);
        uint32_t seq_num_next = get_pdu_seq_num(&sorted_pdu_list[i + 1]);

        // If there's a gap in the sequence numbers, adjust valid_count and break
        if (seq_num_next - seq_num_current != 1)
        {
            valid_count = i + 1; // Include PDUs up to the first gap
            break;
        }
    }

    if (valid_count != pdu_count)
    { // Indication of Gaps?
        printf("GAPS!!!");
    }

    printf("File Validation Complete!");
    return sorted_pdu_list;
}