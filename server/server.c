#include "server.h"
#include <arpa/inet.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

// create a mutex 
pthread_mutex_t file_mutex;

int main() {
    // initialize mutex
    if (pthread_mutex_init(&file_mutex, NULL) != 0) {
        printf("Mutex init failed\n");
        return 1;
    }

    int socket_desc, new_sock, c;
    struct sockaddr_in server, client;

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
    if (listen(socket_desc, 3) < 0) {
        perror("listen failed. Error");
        return 1;
    }
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    // Accept connection from an incoming client
    while (1) {
        new_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_sock < 0) {
            perror("accept failed");
            continue;
        }

        puts("Connection accepted");

        int *thread_sock_ptr = malloc(sizeof(int));
        *thread_sock_ptr = new_sock;

        if (pthread_create(&thread_id, NULL, connection_handler, (void*)thread_sock_ptr) < 0) {
            perror("could not create thread");
            free(thread_sock_ptr);
        }

        // detach the thread so that it cleans up after finishing
        pthread_detach(thread_id);
    }

    // Close the socket before shutting down
    close(socket_desc);
    puts("Server shutting down");
    // destroy mutex
    pthread_mutex_destroy(&file_mutex);

    return 0;
}