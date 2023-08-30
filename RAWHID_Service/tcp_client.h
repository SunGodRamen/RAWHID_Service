#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include "logger.h"

typedef struct {
	const char* ip;
	uint16_t port;
} tcp_socket_info;

int read_message_from_server(SOCKET socket, char* buffer);

// Initialize the client
SOCKET init_client(tcp_socket_info server_info);

// Send data to the server
void send_to_server(SOCKET serverSocket, const char* data, int dataLength);

// Cleanup and close the client
void cleanup_client(SOCKET serverSocket);

#endif
