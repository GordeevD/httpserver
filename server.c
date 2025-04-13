//
// Created by dinn on 4/12/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 2048

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[RESPONSE_SIZE];
    ssize_t bytes_read;

    // Initialize buffers
    memset(buffer, 0, BUFFER_SIZE);
    memset(response, 0, RESPONSE_SIZE);

    // Read the request
    bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Error reading from socket");
        close(client_socket);
        return;
    }

    // Log the request (for debugging purposes)
    printf("Received request:\n%s\n", buffer);

    // Simple HTTP response
    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    const char *body = "Hello, real World! Tuned to the right frequency!\n";

    // Construct the response safely
    if (snprintf(response, RESPONSE_SIZE, "%s%s", header, body) >= RESPONSE_SIZE) {
        fprintf(stderr, "Response buffer overflow detected\n");
        close(client_socket);
        return;
    }

    // Send the response
    if (write(client_socket, response, strlen(response)) < 0) {
        perror("Error writing to socket");
    }

    // Close the connection
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 10) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    // Accept and handle clients
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);
    return 0;
}