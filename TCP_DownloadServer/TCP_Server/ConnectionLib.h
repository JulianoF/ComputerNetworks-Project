#ifndef CONNECTIONLIB_H
#define CONNECTIONLIB_H

#define MAX_PAYLOAD_SIZE 100

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

void closeServer(int sd);

void closeClient(int client_sd);

#endif