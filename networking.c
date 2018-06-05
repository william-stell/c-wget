#include <stdio.h>
#include <string.h>

#include "networking.h"

/**
 * Creates a IPv4 socket based on the given type.
 */
int create_socket(int type) {
    return socket(IPV4, type, 0);
}

/**
 * Start connection to the given address and port.
 */
int start_connection(int socket, const char *address, int port) {
    struct sockaddr_in host;

    host.sin_family = IPV4;
    inet_aton(address, &host.sin_addr);
    host.sin_port = htons(port);

    return connect(socket, (struct sockaddr *)&host, sizeof(host));
}

/**
 * Sends the message from the given buffer to the given socket.
 */
int send_message(int socket, char *buffer) {
    int size = send(socket, buffer, strlen(buffer), 0);

    return size;
}

/**
 * Gets message from the given socket into the given buffer.
 */
int receive_message(int socket, char *buffer) {
    int size = recv(socket, buffer, BUFFER_SIZE, 0);
    buffer[size] = '\0';

    return size;
}

/**
 * Converts the given hostname to ip address.
 */
int get_ip_from_hostname(char *hostname, char *address) {

    struct hostent *hostEntry = gethostbyname(hostname);

    if (hostEntry == NULL) {
        return -1;
    }

    // Convert to ip address
    char *ipAddress = inet_ntoa(*((struct in_addr *)hostEntry->h_addr_list[0]));
    strcpy(address, ipAddress);

    // Check if conversion failed
    if (address == NULL) {
        return -2;
    }

    return 0;
}

