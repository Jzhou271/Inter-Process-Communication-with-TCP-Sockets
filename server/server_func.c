#include "server.h"
#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// Function to create a directory if it does not already exist
void create_dir_if_not_exists(const char *file_path) {
    char *path_copy = strdup(file_path);
    char *dir_path = dirname(path_copy);
    mkdir(dir_path, 0777);
    free(path_copy);
}

// Function to write data received from socket into a file
void write_file(int sock, const char *file_path) {
    // Ensure the directory for the file exists
    create_dir_if_not_exists(file_path);

    // Check if the file already exists
    if (access(file_path, F_OK) != -1) {
        const char *errMsg = "ERROR: File already exists.\n";
        send(sock, errMsg, strlen(errMsg), 0);
        return;
    }

    // Attempt to open the file for writing
    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }

    // Buffer to store data received from the client
    char buffer[1024];
    ssize_t bytes_received;
    // Receive data until there is no more to read
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        // Write the received data to the file
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
    
    const char *successMsg = "File created successfully.\n";
    send(sock, successMsg, strlen(successMsg), 0);

    printf("File sent successfully. Remote path: %s\n", file_path);
}

// Function to send a file to the client
void send_file_to_client(int sock, const char *file_path) {
    // Attempt to open the file for reading
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Failed to open file for reading");
        printf("Attempted to open file: %s\n", file_path);
        const char *errMsg = "ERROR: File does not exist.\n";
        send(sock, errMsg, strlen(errMsg), 0);
        return;
    }

    // Confirm the file was opened successfully
    printf("Successfully opened file: %s for reading.\n", file_path);

    // Buffer to store data read from the file
    char buffer[1024];
    size_t read_bytes;
    // Read from the file and send to the client until there's nothing left to read
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(sock, buffer, read_bytes, 0);
    }

    fclose(file);

    // 打印成功发送文件的消息
    printf("File '%s' sent to client successfully.\n", file_path);
}

// Function to remove a file and notify the client of the result
void remove_file(int sock, const char *file_path) {
    // Check if the file exists
    if (access(file_path, F_OK) != -1) {
        // Attempt to remove the file
        if (remove(file_path) == 0) {
            const char *successMessage = "File removed successfully.\n";
            send(sock, successMessage, strlen(successMessage), 0);
            printf("File '%s' removed successfully.\n", file_path);
        } else {
            // Print the failure to remove the file
            perror("Failed to remove file");
            const char *errorMessage = "ERROR: Failed to remove file.\n";
            send(sock, errorMessage, strlen(errorMessage), 0);
        }
    } else {
        const char *noFileMessage = "No such file.\n";
        send(sock, noFileMessage, strlen(noFileMessage), 0);
        printf("Attempted to remove non-existing file: %s\n", file_path);
    }
}


void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int*)socket_desc;
    free(socket_desc);
    char client_message[2000];

    // Receive a message from client
    ssize_t read_size = recv(sock, client_message, sizeof(client_message) - 1, 0);
    if (read_size > 0) {
        // Null terminate the string
        client_message[read_size] = '\0';

        // Parse the command and the file path from the received message
        char *command = strtok(client_message, " ");
        char *file_path = strtok(NULL, " ");

        // Determine the command (WRITE, GET, RM) and execute the corresponding function
        if (command && strcmp(command, "WRITE") == 0 && file_path) {
            pthread_mutex_lock(&file_mutex);
            write_file(sock, file_path);
            pthread_mutex_unlock(&file_mutex);
        } else if (command && strcmp(command, "GET") == 0 && file_path) {
            pthread_mutex_lock(&file_mutex);
            send_file_to_client(sock, file_path);
            pthread_mutex_unlock(&file_mutex);
        } else if (command && strcmp(command, "RM") == 0 && file_path) {
            pthread_mutex_lock(&file_mutex);
            remove_file(sock, file_path);
            pthread_mutex_unlock(&file_mutex);
        } else {
            // If the command is not supported or the file path is missing
            printf("Unsupported command or missing file path.\n");
        }
    }

    close(sock);
    puts("Client disconnected");
    return NULL;
}
