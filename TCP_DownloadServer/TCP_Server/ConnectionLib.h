#ifndef CONNECTIONLIB_H
#define CONNECTIONLIB_H

#define MAX_PAYLOAD_SIZE 34
//#define MAX_PAYLOAD_SIZE 100 //Uncomment this for 100 byte payload not 100 byte packet

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int initServer(int port, char* ip);

int acceptClient(int sd);

char* handleFilename(int client_sd);

int sendFile(int client_sd, char *filename);

void closeServer(int sd);

void closeClient(int client_sd);

#endif