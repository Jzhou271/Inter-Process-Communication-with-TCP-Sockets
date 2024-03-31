#include "server.h"
#include <arpa/inet.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>


int main() {
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;
    char client_message[2000];

    // Create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    puts("Socket created");

    // Configure settings of the server address struct
    server.sin_family = AF_INET;            // Address family = Internet
    server.sin_addr.s_addr = INADDR_ANY;    // Set IP address to localhost
    server.sin_port = htons(8888);          // Set port number, using htons function to use proper byte order

    // Bind the address struct to the socket
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }

    puts("bind done");

    // Listen on the socket, with 3 max connection requests queued
    listen(socket_desc, 3);
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);

    // Accept connection from an incoming client
    while (1) {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }

        puts("Connection accepted");

        // Receive a message from client
        ssize_t read_size = recv(client_sock, client_message, sizeof(client_message) - 1, 0);
        if (read_size > 0) {
            // Null terminate the string
            client_message[read_size] = '\0';
            // Parse the command and the file path from the received message
            char *command = strtok(client_message, " ");
            char *file_path = strtok(NULL, " ");

            // Determine the command (WRITE, GET, RM) and execute the corresponding function
            if (command && strcmp(command, "WRITE") == 0 && file_path) {
                write_file(client_sock, file_path);
            } else if (command && strcmp(command, "GET") == 0 && file_path) {
                send_file_to_client(client_sock, file_path);
            } else if (command && strcmp(command, "RM") == 0 && file_path) {
                remove_file(client_sock, file_path);
            } else {
                // If the command is not supported or the file path is missing
                printf("Unsupported command or missing file path.\n");
            }
        }

        close(client_sock);
        puts("Client disconnected.");
    }

    close(socket_desc);
    puts("Server shutting down");
    return 0;
}
