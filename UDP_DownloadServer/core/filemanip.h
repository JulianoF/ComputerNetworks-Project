#ifndef FILEMANIPULATION_H
#define FILEMANIPULATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SEQ_LEN 3 //Length of Sequence number (in bytes), 3 bytes or 24 bit
#define DATA_LEN 97
#define PDU_DATA_SIZE 100

struct pdu {
    char type;
    char data[PDU_DATA_SIZE];
};

struct pdu* load_file_into_pdus(const char* filename, int* pdu_count);
int rebuild_file_from_pdus(const char* output_filename, struct pdu* pdu_list, int pdu_count);

struct pdu* validate_pdu_list(struct pdu* dirty_pdu_list, int pdu_count);

#endif