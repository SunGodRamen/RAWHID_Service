#ifndef _RAWHID_H_
#define _RAWHID_H_

#include <hidapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <synchapi.h>
#include <time.h>

hid_device* get_handle(uint16_t vendor_id, uint16_t product_id);
void open_usage_path(uint16_t vendor_id, uint16_t product_id, uint16_t usage_page, uint8_t usage, hid_device** handle);
int write_to_handle(hid_device* handle, unsigned char* message, size_t size);
void print_message(unsigned char* message, size_t size);
void send_ping(hid_device* handle);
bool process_message(hid_device* handle, unsigned char* message, int res, unsigned char* pong);

#endif // _RAWHID_H_