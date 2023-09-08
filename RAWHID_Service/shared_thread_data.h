#ifndef INTERTHREAD_COMM_H
#define INTERTHREAD_COMM_H

#include "message_protocol.h"
#include "logger.h"
#include <stdint.h>
#include "windows.h"

typedef struct {
    unsigned char message_to_tcp[MESSAGE_SIZE_BYTES];
    unsigned char message_from_tcp[MESSAGE_SIZE_BYTES];
    HANDLE mutex;
    HANDLE data_ready_to_send_event;
    HANDLE response_received_event;
} shared_thread_data;

int initialize_shared_data(shared_thread_data* sharedData);
void set_message_to_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size);
void set_message_from_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size);
BOOL check_message_to_tcp(shared_thread_data* sharedData, unsigned char* buffer, size_t size);
BOOL check_message_from_tcp(shared_thread_data* sharedData, unsigned char* buffer, size_t size);
void cleanup_shared_data(shared_thread_data* sharedData);

#endif