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
#include <stdbool.h>

// the head of file lock list
FileLock *fileLocks = NULL;
// the global mutex used to protect the lock list
pthread_mutex_t mapLock = PTHREAD_MUTEX_INITIALIZER;

// function to get file lock for a specific file
FileLock* getFileLock(const char* filepath, char *permission) {
    pthread_mutex_lock(&mapLock);

    FileLock *current = fileLocks;
    while (current != NULL) {
        if (strcmp(current->filepath, filepath) == 0) {
            if (current->permission == -1 && strcmp(permission, "NA") != 0) {
                current->permission = (strcmp(permission, "RO") == 0)? 0 : 1;
            }
            pthread_mutex_unlock(&mapLock);
            return current;
        }
        current = current->next;
    }

    // if the file lock is not found, create a new one
    FileLock *newLock = (FileLock *)malloc(sizeof(FileLock));
    newLock->filepath = strdup(filepath);
    pthread_mutex_init(&(newLock->lock), NULL);
    newLock->next = fileLocks;
    if (strcmp(permission, "NA") == 0) {
        newLock->permission = -1;
    } else {
        newLock->permission = (strcmp(permission, "RO") == 0)? 0 : 1;
    }
    fileLocks = newLock;

    pthread_mutex_unlock(&mapLock);
    return newLock;
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
FILE* create_new_file(const char *file_path, int permission) {
    // if file address has duplicates
    if (access(file_path, F_OK) == 0) {
        
        // if the file is READ-ONLY, return NULL;
        if (permission == 0) {
            printf("[Error]: The file %s is READ-ONLY.\n", file_path);
            return NULL;
        } else {
            // if the file is READ-WRITE
            if (remove(file_path) != 0) {
                perror("Failed to remove exisiting file.\n");
                return NULL;
            } else {
                printf("Duplicated file is found and overwritten.\n");
            }
        }
    }

    // If no duplicated file in the server
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

void clean_command(char *command) {
    int i = 0;
    while (command[i] != '\0') {
        if (command[i] == '\n' || command[i] == '\r' || command[i] == '?') {
            command[i] = '\0';
            break;
        }
        i++;
    }
}

// Function to write data received from socket into a file
void write_file(int sock, char *file_path, char *permission) {
    clean_command(file_path);
    clean_command(permission);

    // get file lock before writing to the file
    FileLock *the_lock = getFileLock(file_path, permission);
    pthread_mutex_t *fileMutex = &(the_lock->lock);
    int perm = the_lock->permission;

    // // check if thread is busy
    bool lockAcquired = false;
    while (!lockAcquired) {
        // if the mutex lock can be required
        if (pthread_mutex_trylock(fileMutex) == 0) {
            lockAcquired = true;
            printf("Lock acquired. The file is ready to be written.\n");
        } else {
            // if the lock is not unlocked yet
            printf("The file %s is being used, waiting to be unlocked...\n", file_path);
            sleep(5);
        }
    }

    // lock is acquired, continue to create file
    FILE *file = create_new_file(file_path, perm);
    if (file == NULL) {
        printf("[Error]: Failed to create a remote file on server.\n");
        pthread_mutex_unlock(fileMutex);
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
    printf("File sent successfully. Remote path: %s\n", file_path);

    // This is for testing mutithread
    // sleep(10);
    pthread_mutex_unlock(fileMutex);
}

// Function to send a file to the client
void send_file_to_client(int sock, const char *file_path) {
    // Attempt to open the file for reading
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        const char *errMsg = "ERROR: File does not exist on server.\n";
        perror(errMsg);
        send(sock, errMsg, strlen(errMsg), 0);
        return;
    }

    const char *okMsg = "OK\n";
    send(sock, okMsg, strlen(okMsg), 0); // means the content of the file will be sent

    // Buffer to store data read from the file
    char buffer[1024];
    size_t read_bytes;
    // Read from the file and send to the client until there's nothing left to read
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(sock, buffer, read_bytes, 0);
    }

    fclose(file);

    printf("File '%s' sent to client successfully.\n", file_path);
}

// Function to remove a file and notify the client of the result
void remove_file(int sock, char *file_path) {
    clean_command(file_path);

    // get file lock before writing to the file
    FileLock *the_lock = getFileLock(file_path, "NA");
    pthread_mutex_t *fileMutex = &(the_lock->lock);

    // check the file permission
    if (the_lock->permission == 0) {
        printf("[Error]: The file %s is READ-ONLY.\n", file_path);
        return;
    }

    // // check if thread is busy
    bool lockAcquired = false;
    while (!lockAcquired) {
        // if the mutex lock can be required
        if (pthread_mutex_trylock(fileMutex) == 0) {
            lockAcquired = true;
            printf("Lock acquired. The file is ready to be removed.\n");
        } else {
            // if the lock is not unlocked yet
            printf("The file %s is being used, waiting to be unlocked...\n", file_path);
            sleep(5);
        }
    }    

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

    pthread_mutex_unlock(fileMutex);
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
        char *permission = strtok(NULL, " ");

        // Determine the command (WRITE, GET, RM) and execute the corresponding function
        if (command && strcmp(command, "WRITE") == 0 && file_path && permission) {
            write_file(sock, file_path, permission);
        } else if (command && strcmp(command, "GET") == 0 && file_path) {
            send_file_to_client(sock, file_path);
        } else if (command && strcmp(command, "RM") == 0 && file_path) {
            remove_file(sock, file_path);
        } else {
            // If the command is not supported or the file path is missing
            printf("Unsupported command or missing file path.\n");
        }
    }

    close(sock);
    puts("Client disconnected");
    return NULL;
}
