#include "client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int main(int argc, char *argv[]) {

    // instruction
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <remote-file-path> <local-file-path> <permission>\n", argv[0]);
        return 1;
    }

    // command checking
    if (strcmp(argv[1], "WRITE") == 0) {
        if (argc < 4) {
            printf("Lack of file permission\n");
            return 1;
        } else if (argc > 5) {
            printf("Too much arguments\n");
            return 1;
        }
    } else if (strcmp(argv[1], "GET") == 0) {
        if (argc < 3) {
            printf("Lack of file permission\n");
            return 1;
        } else if (argc > 4) {
            printf("Too much arguments\n");
            return 1;
        }
    } else if (strcmp(argv[1], "RM") == 0) {
        if (argc < 3) {
            printf("Lack of file permission\n");
            return 1;
        } else if (argc > 3) {
            printf("Too much arguments\n");
            return 1;
        }
    } else {
        printf("Invalid command.\n");
        return 1;
    }

    // permission checking
    if (strcmp(argv[1], "WRITE") == 0 &&
        strcmp(argv[argc - 1], "RO") != 0 && strcmp(argv[argc - 1], "RW") != 0) {
        printf("Invalid file permission.\n");
        return 1;
    }

    int sock;
    struct sockaddr_in server;

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
        return 1;
    }

    puts("Socket created");
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    // server.sin_addr.s_addr = inet_addr("192.168.1.12");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);                     // Server port

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected");

    // Handle "WRITE" command
    if (strcmp(argv[1], "WRITE") == 0) {
        char *local_path = argv[2];
        char *remote_path = argv[argc - 2];
        char *permission = argv[argc - 1];
        send_file(sock, local_path, remote_path, permission);
    }
    // Handle GET command 
    else if (strcmp(argv[1], "GET") == 0) {
        // Determine local file path. If not provided, use the remote file name in the current directory.
        char *remote_path = argv[2];
        char *local_path = argc == 4 ? argv[3] : argv[2];

        // Send the GET command with the remote path to the server
        char buffer[1024];
        sprintf(buffer, "GET %s", remote_path);
        send(sock, buffer, strlen(buffer), 0);
        // Wait and receive the file
        receive_file(sock, local_path);
    }
    // Handle RM command 
    else if (strcmp(argv[1], "RM") == 0) {
        char buffer[1024];
        sprintf(buffer, "RM %s", argv[2]);
        send(sock, buffer, strlen(buffer), 0);
        // send(sock, "\n", 1, 0);

        char response[1024];
        int bytes_received = recv(sock, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
        }
    } else {
        printf("Unsupported command.\n");
    }

    // Close the socket
    close(sock);
    return 0;
}

