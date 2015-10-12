/*
 * client_socket.c
 *
 *  Created on: 05-Oct-2015
 *      Author: siddhanthgupta
 */
#include <stdio.h>              // Apparently, stderr is in stdio.h

#include <sys/socket.h>         // For socket descriptor
#include <sys/errno.h>          // For errno on error
#include <sys/types.h>

#include <netinet/in.h>         // For struct sockaddr_in, htonl, htons functions
#include <unistd.h>             // For read/write functions
#include <string.h>
#include <stdlib.h>
//#include <arpa/inet.h>          // For the htonl function

#define SERVER_PORT_NO 5648
#define MAX_CONCURRENT_CLIENTS 5
#define MAX_BUFFER_SIZE 255

struct client_store {
    int socket;
    char address[MAX_BUFFER_SIZE];
};

struct client_store client_list[MAX_CONCURRENT_CLIENTS + 1];
int client_count = 0;

void display(char* buffer, struct sockaddr_in* from_addr) {
    char s[MAX_BUFFER_SIZE];
    if (inet_ntop(AF_INET, &(from_addr->sin_addr), s, MAX_BUFFER_SIZE) == NULL) {
        fprintf(stderr, "Unable to read address\n");
    } else {
        printf("Message received from %s: %s\n", s, buffer);
    }
}

int add_client(int client_socket, struct sockaddr_in client_address) {
    char s[MAX_BUFFER_SIZE];
    if (inet_ntop(AF_INET, &(client_address.sin_addr), s, MAX_BUFFER_SIZE) == NULL) {
        fprintf(stderr, "Unable to read address\n");
        return -1;
    }
    if (client_count == MAX_CONCURRENT_CLIENTS) {
        fprintf(stderr, "Unable to accept client due to client-limit\n");
        return -1;
    }
    struct client_store* client = client_list + client_count;
    client->socket = client_socket;
    strcpy(client->address, s);
    client_count++;
    return 0;
}

/*
 * On the server side, we need to
 *  Create a socket using the socket() call
 *  Bind the socket to an address using the bind() call
 *  Listen for connections using listen() call
 *  Accept a connection using accept() system call. This causes the server to be
 *  blocked, until the 3-way TCP handshake is complete
 *
 *  To accept connections, the following steps are performed:
 *
 *      1.  A socket is created with socket(2).
 *
 *      2.  The socket is bound to a local address using bind(2),  so  that
 *          other sockets may be connect(2)ed to it.
 *
 *      3.  A  willingness to accept incoming connections and a queue limit
 *          for incoming connections are specified with listen().
 *
 *      4.  Connections are accepted with accept(2).
 *
 *
 */
int main(int argc, char** argv) {
    // Creating a socket: IPv4 domain, TCP connection oriented type
    // Protocol for this is TCP by default
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s: Error: Unable to create server socket. Exiting.\n", argv[0]);
        exit(1);
    }

    // The socket is bound to an address using the bind() call
    // We define an address
    struct sockaddr_in address, client_address;
    int client_address_length, client_socket;
    char buffer[MAX_BUFFER_SIZE];
    int bytes_read;
    address.sin_family = AF_INET;
    // For the Internet domain, if we specify the special IP address INADDR_ANY (defined in
    // <netinet/in.h>), the socket endpoint will be bound to all the system’s network
    // interfaces. This means that we can receive packets from any of the network interface
    // cards installed in the system.
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT_NO);
    if (bind(server_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
        fprintf(stderr, "%s: Error: Unable to bind server socket.\n", argv[0]);
        exit(1);
    }

    // We listen for connections now
    if (listen(server_socket, MAX_CONCURRENT_CLIENTS) < 0) {
        fprintf(stderr, "%s: Error: Unable to listen for connections.\n", argv[0]);
        exit(1);
    }
    // signal(SIGINT, delete_socket);
    // We accept connections indefinitely in a loop
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr*) &client_address,
                (socklen_t*) &client_address_length)) < 0) {
            fprintf(stderr, "%s: Error: Unable to accept connection.\n", argv[0]);
            exit(1);
        }
        if (add_client(client_socket, client_address) < 0) {
            fprintf(stderr, "%s: Error: Unable to create entry in client table.\n", argv[0]);
            exit(1);
        }
        // Clearing buffer and reading from client socket
        pid_t pid2 = fork();
        if (pid2 < 0) {
            fprintf(stderr, "%s: Error: Unable to fork listener process.\n", argv[0]);
            exit(1);
        }
        if (pid2 == 0) {
            // Child Process
            int s = client_socket;
            while (1) {
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                memset(buffer, 0, MAX_BUFFER_SIZE);
                if ((bytes_read = read(s, buffer, MAX_BUFFER_SIZE)) <= 0) {
                    fprintf(stderr, "%s : Error: No data read from client.\n", argv[0]);
                    exit(1);
                }
                display(buffer, &client_address);
            }
            exit(0);
        }
        if (client_count == 1) {
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "%s: Error: Unable to fork listener process.\n", argv[0]);
                exit(1);
            }
            if (pid == 0) {
                // Child Process
                while (1) {
                    char str[255];
                    scanf("%s", str);
                    write(client_list[*str - '0'].socket, str + 2, 253);
                }
                exit(0);
            }
        }

    }
    return 0;
}

