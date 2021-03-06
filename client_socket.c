/*
 * server_socket.c
 *
 *  Created on: 05-Oct-2015
 *      Author: siddhanthgupta
 */
#include <sys/socket.h>         // For socket
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <netinet/in.h>
#include <netdb.h>              // For gethostbyname
#define SERVER_PORT_NO 5648
void usage(char* program) {
    printf("USAGE: %s <Server IP> <Port Number>\n", program);
}

/*
 * The steps involved in establishing a socket on the client side are as follows:
 *
 *     1)   Create a socket with the socket() system call
 *     2)   Connect the socket to the address of the server using the connect() system call
 *     3)   Send and receive data. There are a number of ways to do this, but the simplest
 *          is to use the read() and write() system calls.
 *
 */
int main(int argc, char** argv) {
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }
    int client_socket;
    struct hostent *server;
    struct sockaddr_in server_address;
    // Creating the socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s: Error: Unable to create socket.\n", argv[0]);
        exit(1);
    }

    // Setting server address using the IP provided by the user
    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "%s: Error: Invalid host.\n", argv[0]);
        exit(1);
    }
    server_address.sin_family = AF_INET;
    memcpy((char*) &(server_address.sin_addr.s_addr), (char*) server->h_addr_list[0],
            (size_t) server->h_length);
    server_address.sin_port = htons(SERVER_PORT_NO);

    // Connecting to server
    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "%s: Error: Unable to connect to server.\n", argv[0]);
        // TODO: More meaningful error message
        exit(1);
    }
 //  while (1) {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "%s: Error: Unable to fork listener process.\n", argv[0]);
            exit(1);
        } else if (pid == 0) {
            while (1) {
                char str[255];
                scanf("%s", str);
                write(client_socket, str, 255);
            }
            exit(0);
        }
        else {
            // Child Process
            while(1) {
                char buffer[255];
                int bytes_read;
                memset(buffer, 0, 255);
                if ((bytes_read = read(client_socket, buffer, 255)) <= 0) {
                    fprintf(stderr, "%s : Error: No data read from client.\n", argv[0]);
                    exit(1);
                }
                printf("Received: %s\n", buffer);
            }
        }
 //   }
    return 0;
}

