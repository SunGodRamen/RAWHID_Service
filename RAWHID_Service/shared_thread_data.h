#ifndef INTERTHREAD_COMM_H
#define INTERTHREAD_COMM_H

#include "message_protocol.h"
#include <stdint.h>
#include "windows.h"

typedef struct {
    unsigned char message_to_tcp[MESSAGE_SIZE_BITS];
    uint64_t message_from_tcp;
    HANDLE mutex;
    HANDLE data_ready_event;
} shared_thread_data;

int initialize_shared_data(shared_thread_data* sharedData);
void set_message_to_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size);
void set_message_from_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size);
void cleanup_shared_data(shared_thread_data* sharedData);

#endif