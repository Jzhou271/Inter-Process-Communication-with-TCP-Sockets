#include "../header/client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// Function to check if a file exists
bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// Function to create a new file
void create_file(char *filepath) {
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Failed to create file");
        exit(1);
    }
    fclose(file);
    printf("Created %s file successfully\n", filepath);
}

// Function to send a file to the server
void send_file(int sock, char *local_path, char *remote_path) {
    // Attempt to create the file only if it does not exist
    if (!file_exists(local_path)) {
        create_file(local_path);
        printf("File %s created successfully. Preparing to send...\n", local_path);

        FILE *file = fopen(local_path, "rb");
        if (file == NULL) {
            perror("Failed to open file");
            exit(1);
        }

        char buffer[1024];
        sprintf(buffer, "WRITE %s", remote_path);
        send(sock, buffer, strlen(buffer), 0);

        size_t read_bytes;
        while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(sock, buffer, read_bytes, 0);
        }

        fclose(file);
        printf("File sent successfully. Local path: %s, Remote path: %s\n", local_path, remote_path);
    } else {
        // If the file already exists, don't send it.
        printf("File %s already exists. Not sending.\n", local_path);
    }
}

// Function to receive a file from the server
void receive_file(int sock, char *local_path) {
    char buffer[1024];
    int bytes_received;
    FILE *file = fopen(local_path, "wb");
    if (file == NULL) {
        perror("Failed to open local file");
        exit(1);
    }

    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received == 0) {
        printf("File received successfully.\n");
    } else {
        printf("Error receiving file or connection closed.\n");
    }

    fclose(file);
}

