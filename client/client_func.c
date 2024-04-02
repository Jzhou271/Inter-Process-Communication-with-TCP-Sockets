#include "client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

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

    // If the file exists on the client 
    // we will handle the file overwrite on the server side if file already exist on server 
    if (file_exists(local_path)) {

        FILE *file = fopen(local_path, "rb");
        if (file == NULL) {
            perror("Failed to open file");
            exit(1);
        }

        char buffer[1024];
        sprintf(buffer, "WRITE %s\n", remote_path);
        send(sock, buffer, strlen(buffer), 0);

        size_t read_bytes;
        while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(sock, buffer, read_bytes, 0);
        }

        fclose(file);
        printf("File sent successfully from client to server.\n");
    } else {
        // if the local file does not exit, return error
        printf("[ERROR]: File is NOT found on client.\n");
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

void create_dir_for_path(const char *file_path) {
  char *path_copy = strdup(file_path); // Create a modifiable copy of file_path
  char *dir_path = dirname(path_copy); // Extract the directory path

  struct stat st = {0};
  if (stat(dir_path, &st) == -1) {
    mkdir(dir_path, 0777); // Create the directory if it doesn't exist
  }

  free(path_copy); // Free the duplicated string
}

