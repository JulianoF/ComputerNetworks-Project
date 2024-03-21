#ifndef FILEMANIPULATION_H
#define FILEMANIPULATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEQ_LEN 2 //Length of Sequence number (in bytes), 2 bytes or 16 bits can support 65K Frames
#define DATA_LEN 98
#define PDU_DATA_SIZE 100

struct pdu {
    char type;
    char data[PDU_DATA_SIZE];
};

struct pdu* load_file_into_pdus(const char* filename, int* pdu_count);
int rebuild_file_from_pdus(const char* output_filename, struct pdu* pdu_list, int pdu_count);

#endif