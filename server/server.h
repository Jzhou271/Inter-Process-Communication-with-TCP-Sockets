#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// create a mutex 
extern pthread_mutex_t file_mutex;

// Function to create a directory if it does not already exist
void create_dir_if_not_exists(char *file_path);

// Function to write data received from socket into a file
void write_file(int sock, char *file_path);

// Function to send a file to the client
void send_file_to_client(int sock, const char *file_path);

// Function to remove a file and notify the client of the result
void remove_file(int sock, const char *file_path);

// handle commands sent from clients
void *connection_handler(void *socket_desc);

#endif // SERVER_H
