#include "rawhid.h"

hid_device* get_handle(uint16_t vendor_id, uint16_t product_id) {
    hid_device* handle = hid_open(vendor_id, product_id, NULL);
    return handle;
}

void open_usage_path(uint16_t vendor_id, uint16_t product_id, uint16_t usage_page, uint8_t usage, hid_device** handle) {
    if (*handle) {
        struct hid_device_info* info = hid_enumerate(vendor_id, product_id);
        // Loop through device info list
        for (; info != NULL; info = info->next) {
            if (info->usage_page == usage_page && info->usage == usage) {
                *handle = hid_open_path(info->path);
                if (*handle) {
                    wprintf(L"  Successfully opened device\n");
                    break;
                }
            }
        }
        hid_free_enumeration(info);
    }
}

bool process_message(hid_device* handle, unsigned char* message, int res, unsigned char* pong) {
    if (res < 0) {
        fwprintf(stderr, L"Failed to read response: %ls\n", hid_error(handle));
        Sleep(500);
    }
    else if (res > 0) {
        if (res >= sizeof(pong) && memcmp(message, pong, sizeof(pong)) == 0) {
            wprintf(L"PONG\n");
            return true;
        }
        wprintf(L"Found message: ");
        print_message(message, res);
        return true;
    }
    return false;
}

// Function to write to the handle
int write_to_handle(hid_device* handle, unsigned char* message, size_t size) {
    return hid_write(handle, message, size);
}

void print_message(unsigned char* message, size_t size) {
    for (size_t i = 0; i < size; i++) {
        fprintf(stderr, "%02X ", message[i]);
    }
    fprintf(stderr, "\n");
}

// Function to send a ping message
void send_ping(hid_device* handle) {
    // pings are 0x01
    // first byte seems to be unrecognized on keyboard
    unsigned char ping[64] = { 0x00, 0x01 };
    if (write_to_handle(handle, ping, sizeof(ping)) < 0) {
        wprintf(stderr, "Failed to send ping: %s\n", hid_error(handle));
    }
    else {
        fprintf(stderr, "PING\n");
        //print_message(ping, sizeof(ping));
    }
}
