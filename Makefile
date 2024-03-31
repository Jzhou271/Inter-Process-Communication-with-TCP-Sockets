CC = gcc
CFLAGS = -Wall -Iheader
SRC_DIR = source
HEADER_DIR = header
MAIN_DIR = main
# Update executable files list
EXECUTABLES = client server

# Source files
SRCS = $(SRC_DIR)/client_func.c
SERVER_SRCS = $(SRC_DIR)/server_func.c

# Default target
all: $(EXECUTABLES)

# Compile and link each executable
client: $(MAIN_DIR)/client.c $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

server: $(MAIN_DIR)/server.c $(SERVER_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean up
clean:
	rm -f $(EXECUTABLES) $(SRC_DIR)/*.o disk/*.txt
