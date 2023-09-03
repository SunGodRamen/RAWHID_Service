#ifndef RAWHID_THREAD_H
#define RAWHID_THREAD_H

#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include "rawhid.h"
#include "message_protocol.h"
#include "shared_thread_data.h"
#include "logger.h"

// Structure to hold information required for HID device usage.
typedef struct {
    hid_usage_info* device_info;
    shared_thread_data* shared_data;
} hid_thread_config;

DWORD WINAPI rawhid_device_thread(LPVOID thread_config);

#endif