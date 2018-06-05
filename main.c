#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "networking.h"

// Error codes
#define USAGE_ERROR 1
#define HOSTNAME_ERROR 2
#define PROTOCOL_ERROR 3
#define FILE_NOT_FOUND_ERROR 4
#define CONNECTION_FAILED 5

// Function prototypes
void print_error_message(int errorCode);
void send_http_request(int socket, char *directory, char *filename, 
        char *hostname);
int get_http_status(const char *buffer);
int get_content_length(const char *buffer);


int main(int argc, char *argv[]) {

    // Check program arguments
    if (argc != 2) {
        print_error_message(USAGE_ERROR);
        return USAGE_ERROR;
    }

    // Buffer
    char buffer[BUFFER_SIZE + 1];

    // Store protocol
    char protocol[10];

    // Server address and port
    char hostname[1024];
    int port = 80;

    // Store directory and filename
    char directory[1024];
    char filename[1024];

    // Get protocol
    for (int i = 0; i < strlen(argv[1]); i++) {
        // If it found "://"
        if (argv[1][i] == ':' && argv[1][i + 1]  == '/' 
                && argv[1][i + 2] == '/') {
            // Store protocol
            strcpy(protocol, argv[1]);
            protocol[i] = '\0';

            // Store hostname
            strcpy(hostname, argv[1] + i + 3);

            break;
        }
    }

    // Check if protocol is not found
    if (hostname == NULL) {
        print_error_message(PROTOCOL_ERROR);
        return PROTOCOL_ERROR;
    }

    // Check if it is not http or https protocol
    if (strcmp(protocol, "http") && strcmp(protocol, "https")) {
        print_error_message(PROTOCOL_ERROR);
        return PROTOCOL_ERROR;
    }

    // Get directory from hostname string
    for (int i = 0; i < strlen(hostname); i++) {
        // Check for the first '/' from the hostname
        if (hostname[i] == '/') {
            // Get directory from hostname
            strcpy(directory, hostname + i);

            // Remove directory from hostname
            hostname[i] = '\0';

            break;
        }
    }

    // Get filename from directory string
    filename[0] = '\0';
    for (int i = strlen(directory) - 1; i >= 0; i--) {
        // Check for the last '/'
        if (directory[i] == '/') {
            // Get filename from hostname
            strcpy(filename, directory + i + 1);

            // Remove the filename at the end of the directory string
            directory[i + 1] = '\0';

            break;
        }
    }

    // Check if there is no filename -> exit with error message
    if (strlen(filename) == 0) {
        print_error_message(FILE_NOT_FOUND_ERROR);
        return FILE_NOT_FOUND_ERROR;
    }
    
    // IP address
    char ipAddress[16];
    
    // Get IP address
    if (get_ip_from_hostname(hostname, ipAddress) < 0) {
        print_error_message(HOSTNAME_ERROR);
        return HOSTNAME_ERROR;
    }


    // --- Start server connection --- //

    // Create socket
    int sock = create_socket(TCP);

    // Start connection
    if (start_connection(sock, ipAddress, port) < 0) {
        print_error_message(CONNECTION_FAILED);
        return CONNECTION_FAILED;
    }   

    // Send http request
    send_http_request(sock, directory, filename, hostname);

    // Create a new file (overwrites it)
    FILE *file = fopen(filename, "wb");
    fclose(file);

    // Open file again to start appending contents
    file = fopen(filename, "ab");
    
    int size = 0;
    int isGetReply = 1;

    int contentSize = 0;
    int totalSize = 0;

    // Get message from the socket
    while ((size = receive_message(sock, buffer)) > 0) {

        // First message receive will contain http response
        if (isGetReply) {
            // Check if the file does not exist (404 error)
            if (get_http_status(buffer) == 404) {
                printf("404 error\n");
                print_error_message(FILE_NOT_FOUND_ERROR);
                return FILE_NOT_FOUND_ERROR;
            }
            
            // Get the data content length
            contentSize = get_content_length(buffer);

            // Loop through all characters
            for (int i = 0; i < size - 1; i++) {

                // Find a blank line for the end of http response
                if (buffer[i] == '\n' && buffer[i + 1] == '\r') {

                    isGetReply = 0;

                    // Remove http response message
                    strcpy(buffer, buffer + i + 3);
                    
                    totalSize += strlen(buffer);

                    // Done finding the blank line
                    break;
                }
            }

        } else {
            // Add to total size
            totalSize += size;            
        }

        // Write the contents to the file
        fprintf(file, "%s", buffer);
        fflush(file);

        // Close connection when all of the bytes have been received
        if (totalSize >= contentSize) {
            shutdown(sock, 2);
            break;
        }
    }

    // Close the file
    fclose(file);

    printf("File downloaded as: %s\n", filename);
    printf("Connection closed\n");

    return 0;
}

/**
 * Prints the error message based on the given error code.
 */
void print_error_message(int errorCode) {
    char message[1024];

    switch (errorCode) {
        case USAGE_ERROR:
            sprintf(message, "Usage: webget PROTOCOL://HOST/FILENAME\n");
            break;
        case HOSTNAME_ERROR:
            sprintf(message, "Invalid hostname\n");
            break;
        case PROTOCOL_ERROR:
            sprintf(message, "Invalid protocol\n");
            break;
        case FILE_NOT_FOUND_ERROR:
            sprintf(message, "File not found\n");
            break;
        case CONNECTION_FAILED:
            sprintf(message, "Connection failed\n");
            break;
    }
    fprintf(stderr, "%s", message);
}

/**
 * Sends a http request based on the given values.
 */
void send_http_request(int socket, char *directory, char *filename, 
        char *hostname) {

    char buffer[BUFFER_SIZE];

    // Send a message
    sprintf(buffer, "GET %s%s HTTP/1.1\r\n", directory, filename);
    send_message(socket, buffer);

    sprintf(buffer, "Host: %s\r\n", hostname);
    send_message(socket, buffer);

    send_message(socket, "User-Agent: Firefox/3.6.10\r\n");

    send_message(socket, "Keep-Alive: 115\r\n");
    send_message(socket, "Connection: Keep-Alive\r\n");

    // End of GET request
    send_message(socket, "\r\n");
}

/**
 * Gets http code status from the buffer.
 */
int get_http_status(const char *buffer) {
    char *ret = strstr(buffer, "HTTP/1.1 ");

    char number[4];
    strncpy(number, ret + strlen("HTTP/1.1 "), 3);

    return strtol(number, NULL, 10);
}

/**
 * Gets content length from the buffer.
 */
int get_content_length(const char *buffer) {
    char *ret = strstr(buffer, "Content-Length: ");
    
    for (int i = 0; i < strlen(ret); i++) {
        if (ret[i] == '\n') {
            ret[i] = '\0';
        }
    }
    char number[10];
    strcpy(number, ret + strlen("Content-Length: "));

    return strtol(number, NULL, 10);
}
