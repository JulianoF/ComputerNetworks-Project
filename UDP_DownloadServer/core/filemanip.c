#include "filemanip.h"

uint32_t get_pdu_size(struct pdu *pdu)
{
    uint32_t PDU_S = (pdu->data[0] << 16) | (pdu->data[1] << 8) | pdu->data[2];
    return PDU_S;
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

        // Pack 24 least significant bits of seq_num into pdus[i].data[0], [1], [2]

        pdus[i].data[0] = (char)((seq_num >> 16) & 0xFF); // Extract bits 17-24 (msb)
        pdus[i].data[1] = (char)((seq_num >> 8) & 0xFF);  // Extract bits 9-16
        pdus[i].data[2] = (char)(seq_num & 0xFF);         // Extract bits 1-8 (lsb)

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

    // Bubble sort algorithm
    for (int i = 0; i < pdu_count - 1; i++)
    {
        for (int j = 0; j < pdu_count - i - 1; j++)
        {

            uint32_t seq_num_j = get_pdu_size(&sorted_pdu_list[j]);
            uint32_t seq_num_j1 = get_pdu_size(&sorted_pdu_list[j + 1]);

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
        uint32_t seq_num_current = get_pdu_size(&sorted_pdu_list[i]);
        uint32_t seq_num_next = get_pdu_size(&sorted_pdu_list[i + 1]);

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