#ifndef INTERTHREAD_COMM_H
#define INTERTHREAD_COMM_H

#include <stdint.h>
#include "windows.h"

#ifndef MESSAGE_SIZE_BITS
#define MESSAGE_SIZE_BITS 64
#endif

typedef struct {
    unsigned char message_to_tcp[MESSAGE_SIZE_BITS];
    uint64_t message_from_tcp;
    HANDLE mutex;
    HANDLE data_ready_event;  // event that signals that new data is available
} SharedData;

extern SharedData sharedData;

#endif