#ifndef NETWORKING_H
#define NETWORKING_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP SOCK_STREAM
#define UDP SOCK_DGRAM

#define IPV4 AF_INET

#define BUFFER_SIZE 1024

int create_socket(int type);
int start_connection(int socket, const char *address, int port);

int send_message(int socket, char *buffer);
int receive_message(int socket, char *buffer);

int get_ip_from_hostname(char *hostname, char *address);

#endif
