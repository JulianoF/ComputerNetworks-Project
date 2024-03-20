#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER "127.0.0.1" 
#define BUFLEN 512  
#define PORT 8888   

void killserver(char *s) {
    perror(s);
    exit(1);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_socket_ADDR;
    int s, slen=sizeof(server_socket_ADDR);
    char buf[BUFLEN];
    char message[BUFLEN];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host_ip> [port]\n", argv[0]);
        exit(1);
    }

    int port = (argc == 3) ? atoi(argv[2]) : PORT;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        killserver("socket");
    }

    memset((char *) &server_socket_ADDR, 0, sizeof(server_socket_ADDR));
    server_socket_ADDR.sin_family = AF_INET;
    server_socket_ADDR.sin_port = htons(port);
     
    if (inet_aton(argv[1] , &server_socket_ADDR.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    while(1) {
        printf("Enter message: ");
        gets(message);
        
        // Send the message
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &server_socket_ADDR, slen) == -1) {
            killserver("sendto()");
        }
        
        // Receive a reply and print it
        // Clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        // Try to receive some data, this is a blocking call
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &server_socket_ADDR, &slen) == -1) {
killserver("recvfrom()");
}    printf("Server reply: %s\n", buf);
}

close(s);
return 0;
}
