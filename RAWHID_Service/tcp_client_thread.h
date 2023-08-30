#ifndef TCP_CLIENT_THREAD_H
#define TCP_CLIENT_THREAD_H

#include <stdbool.h>
#include "tcp_client.h"
#include "message_protocol.h"
#include "interthread_comm.h"
#include "logger.h"

DWORD WINAPI tcp_client_thread(LPVOID server_info);

#endif // !define TCP_CLIENT_THREAD_H

