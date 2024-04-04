#ifndef CLINET_H
#define CLINET_H

#include <stdbool.h>

// Function to check if a file exists
bool file_exists(char *filename);

// Function to create a new file
void create_file(char *filepath);

// Function to send a file to the server
void send_file(int sock, char *local_path, char *remote_path, char *permission);

// Function to receive a file from the server
void receive_file(int sock, char *local_path);

// Function to check file permission from the server
void receive_permission(int sock);

void create_dir_for_path(const char *file_path);

#endif // CLINET_H
