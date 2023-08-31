#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include "logger.h"

// Structure to hold information required for TCP socket connection
typedef struct {
	const char* ip;  // IP address of the server
	uint16_t port;   // Port number to connect to
} tcp_socket_info;

// Function prototypes
int read_message_from_server(SOCKET socket, char* buffer);
SOCKET init_client(tcp_socket_info server_info);
void send_to_server(SOCKET serverSocket, const char* data, int dataLength);
void cleanup_client(SOCKET serverSocket);

#endif
