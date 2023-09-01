#ifndef TCP_CLIENT_THREAD_H
#define TCP_CLIENT_THREAD_H

#include <stdbool.h>
#include "tcp_client.h"
#include "message_protocol.h"
#include "shared_thread_data.h"
#include "logger.h"

typedef struct {
    tcp_socket_info* server_config;
    shared_thread_data* shared_data;
} client_thread_config;

DWORD WINAPI tcp_client_thread(LPVOID server_info);

#endif // TCP_CLIENT_THREAD_H

