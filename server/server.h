/*
 * server.h / Practicum 2
 *
 * Kaicheng Jia & Jing Zhou / CS5600 / Northeastern University
 * Spring 2024 / Apr 8, 2024
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct FileLock {
    char *filepath;
    pthread_mutex_t lock;
    struct FileLock *next;
    int permission; // 0 is read only, 1 is read-write, -1 is NA
} FileLock;

typedef struct {
  char grid[5][5];
} PolybiusTable_t;

// the head of file lock list
extern FileLock *fileLocks;

// the global mutex used to protect the lock list
extern pthread_mutex_t mapLock; 

// Function to write data received from socket into a file
void write_file(int sock, char *file_path, char *permission);

// Function to send a file to the client
void send_file_to_client(int sock, const char *file_path);

// Function to remove a file and notify the client of the result
void remove_file(int sock, char *file_path);

// handle commands sent from clients
void *connection_handler(void *socket_desc);

// find the file lock for a specific file
FileLock *getFileLock(const char* filepath, char *permission);

// initialize the Polybius table
void initializePolybiusTable(PolybiusTable_t *table);

// Function to encode plaintext
char *pbEncode(const char *plaintext, const PolybiusTable_t *table);

// Function to decode plaintext
char *pbDecode(const char *ciphertext, const PolybiusTable_t *table);


#endif // SERVER_H
