#ifndef _RAWHID_H_
#define _RAWHID_H_

#include <hidapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <synchapi.h>
#include <time.h>
#include "logger.h"
#include "interthread_comm.h"

#define PING_INTERVAL 10

typedef struct {
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t usage_page;
    uint8_t usage;
} hid_usage_info;

hid_device* get_handle(struct hid_usage_info* device_info);
void open_usage_path(struct hid_usage_info* device_info, hid_device** handle);
int write_to_handle(hid_device* handle, unsigned char* message, size_t size);

#endif // _RAWHID_H_