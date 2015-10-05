/*
 * server_socket.c
 *
 *  Created on: 05-Oct-2015
 *      Author: siddhanthgupta
 */
#include <sys/socket.h>         // For socket
//#include <netinet/in.h>
#include <netdb.h>              // For gethostbyname

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
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s: Error: Unable to create socket.\n", argv[0]);
        exit(1);
    }
    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "%s: Error: Invalid host.\n");
        exit(1);
    }

}

