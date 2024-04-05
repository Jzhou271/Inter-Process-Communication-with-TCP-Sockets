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
#include <errno.h>

// Function to check if a file exists
bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// recursively create directory if its path does not exist
void create_dir_recursive(char *dir_path) {
    if (access(dir_path, F_OK) == 0) {
        // Path already exists
        return;
    }
    char *path_copy = strdup(dir_path);
    if (!path_copy) {
        perror("Failed to duplicate path");
        return;
    }
    for (char *p = strchr(path_copy + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
        if (mkdir(path_copy, 0755) && errno != EEXIST) {
            perror("Failed to create directory");
            free(path_copy);
            return;
        }
        *p = '/';
    }
    if (mkdir(path_copy, 0755) && errno != EEXIST) {
        perror("Failed to create directory");
    }
    free(path_copy);
}

// get address of the file to be written
FILE* create_new_file(const char *file_path) {
    // if file address has duplicates
    if (access(file_path, F_OK) == 0) {
        if (remove(file_path) != 0) {
            perror("Failed to remove exisiting file.\n");
            return NULL;
        } else {
            printf("Duplicated file is found and overwritten.\n");
        }
    }

    // Separate directory and base filename
    char *path_copy = strdup(file_path);
    if (!path_copy) {
        perror("Failed to duplicate file_path");
        return NULL;
    }
    char *dir_path = dirname(path_copy);

    // Create directory recursively if it doesn't exist
    create_dir_recursive(dir_path);

    // Now, try to open the file
    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("Failed to open file");
    }

    free(path_copy);
    return file;
}

void clean_filename(char *filename) {
    int i = 0;
    while (filename[i] != '\0') {
        if (filename[i] == '\n' || filename[i] == '\r' || filename[i] == '?') {
            filename[i] = '\0';
            break;
        }
        i++;
    }
}

// Function to send a file to the server
void send_file(int sock, char *local_path, char *remote_path, char *permission) {

    // If the file exists on the client 
    // we will handle the file overwrite on the server side if file already exist on server 
    if (file_exists(local_path)) {

        FILE *file = fopen(local_path, "rb");
        if (file == NULL) {
            perror("Failed to open file");
            exit(1);
        }

        char buffer[1024];
        sprintf(buffer, "WRITE %s %s\n", remote_path, permission);
        send(sock, buffer, strlen(buffer), 0);

        size_t read_bytes;
        while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(sock, buffer, read_bytes, 0);
        }

        fclose(file);
        printf("File sent successfully from client to server.\n\n");
    } else {
        // if the local file does not exit, return error
        printf("[ERROR]: File is NOT found on client.\n");
    }
}

// Function to receive a file from the server
void receive_file(int sock, char *local_path) {
    clean_filename(local_path);

    // Check the status message first
    char statusMsg[1024] = {0};
    int total_bytes_received = 0, bytes_received;
    
    while (total_bytes_received < 3) { 
        bytes_received = recv(sock, statusMsg + total_bytes_received, 3 - total_bytes_received, 0);
        if (bytes_received <= 0) {
            printf("Error receiving status message or connection closed.\n");
            return;
        }
        total_bytes_received += bytes_received;
    }

    // Ensure the message is null-terminated and checks the status message
    statusMsg[total_bytes_received] = '\0'; // Adjust based on the actual bytes received

    // If we don't receive an OK
    if (strncmp(statusMsg, "OK\n", 3) != 0) { // Use strncmp for safety
        printf("ERROR: File does not exist on server.\n");
        return;
    }

    // If we receive an OK, continue to create file
    FILE *file = create_new_file(local_path);
    if (file == NULL) {
        perror("Failed to create a new file on client.\n");
        return;
    }

    // Receive the file content
    char buffer[1024];
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    // Finalize
    if (bytes_received < 0) {
        printf("Error receiving file.\n");
    } else if (bytes_received == 0) {
        printf("File received successfully.\n");
    }

    fclose(file);
}

// Function to check file permission from the server
void receive_permission(int sock) {
    char buffer[1024] = {0};
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        perror("recv failed or connection closed");
        return;
    }
    buffer[bytesReceived] = '\0';
    printf("%s\n", buffer);
}
