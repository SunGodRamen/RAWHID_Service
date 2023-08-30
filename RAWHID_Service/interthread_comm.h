#ifndef INTERTHREAD_COMM_H
#define INTERTHREAD_COMM_H

#include <stdint.h>
#include "windows.h"

#define MAX_MESSAGE_SIZE 64

typedef struct {
    unsigned char message_to_tcp[MAX_MESSAGE_SIZE];
    uint64_t message_from_tcp;
    HANDLE mutex;
    HANDLE data_ready_event;  // event that signals that new data is available
} SharedData;

extern SharedData sharedData;

#endif