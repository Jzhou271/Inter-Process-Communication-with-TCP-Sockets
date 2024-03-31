CC = gcc
CFLAGS = -Wall -Iheader

# Update executable files list
EXECUTABLES = client/client server/server

# Default target
all: $(EXECUTABLES)

# Compile and link each executable
client/client: client/client.c client/client_func.c client/client.h
	$(CC) $(CFLAGS) client/client.c client/client_func.c -o client/client 

server/server: server/server.c server/server_func.c server/server.h
	$(CC) $(CFLAGS) server/server.c server/server_func.c -o server/server

# Clean up
clean:
	rm -f $(EXECUTABLES) client/local/* server/remote/*
