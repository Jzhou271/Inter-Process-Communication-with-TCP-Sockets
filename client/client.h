/*
 * client.h / Practicum 2
 *
 * Kaicheng Jia & Jing Zhou / CS5600 / Northeastern University
 * Spring 2024 / Apr 8, 2024
 *
 */

#ifndef CLINET_H
#define CLINET_H

#include <stdbool.h>
#include <stdio.h>

// Function to check if a file exists
bool file_exists(char *filename);

// Function to create a new file
void create_dir_recursive(char *dir_path);

// get address of the file to be written
FILE* create_new_file(const char *file_path);

// Function to send a file to the server
void send_file(int sock, char *local_path, char *remote_path, char *permission);

// Function to receive a file from the server
void receive_file(int sock, char *local_path);

// Function to check file permission from the server
void receive_permission(int sock);

#endif // CLINET_H
