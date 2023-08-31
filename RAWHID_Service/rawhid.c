#include "rawhid.h"

#ifndef MESSAGE_SIZE_BITS
#define MESSAGE_SIZE_BITS 64
#endif

/**
 * Opens a HID device based on vendor and product IDs.
 *
 * @param usage_info Pointer to a hid_usage_info struct containing device details.
 * @return A handle to the HID device, or NULL if an error occurs.
 */
hid_device* get_handle(hid_usage_info* usage_info) {
    if (!usage_info) {
        write_log("Usage info is NULL");
        return NULL;
    }

    hid_device* handle = hid_open(usage_info->vendor_id, usage_info->product_id, NULL);
    if (!handle) {
        write_log("Failed to get handle");
    }

    return handle;
}

/**
 * Opens a HID device based on its usage path.
 *
 * @param usage_info Pointer to a hid_usage_info struct containing device details.
 * @param handle Double pointer to the handle where the HID device handle will be stored.
 */
void open_usage_path(hid_usage_info* usage_info, hid_device** handle) {
    if (!handle || !usage_info) {
        write_log("Invalid arguments");
        return;
    }

    if (*handle == NULL) {
        write_log("No handle to open");
        return;
    }

    struct hid_device_info* enum_device_info = hid_enumerate(&usage_info->vendor_id, &usage_info->product_id);
    if (!enum_device_info) {
        write_log("Failed to enumerate devices");
        return;
    }

    for (; enum_device_info != NULL; enum_device_info = enum_device_info->next) {
        if (enum_device_info->usage_page == usage_info->usage_page &&
            enum_device_info->usage == usage_info->usage) {

            *handle = hid_open_path(enum_device_info->path);
            if (*handle) {
                write_log("Successfully opened device");
                break;
            }
        }
    }

    hid_free_enumeration(enum_device_info);
}

/**
 * Writes a message to the given HID handle.
 *
 * @param handle The handle to the HID device.
 * @param message Pointer to the message buffer.
 * @param size The size of the message in bytes.
 * @return The number of bytes written, or -1 if an error occurs.
 */
int write_to_handle(hid_device* handle, unsigned char* message, size_t size) {
    if (!handle || !message) {
        write_log("Invalid arguments");
        return -1;
    }

    int result = hid_write(handle, message, size);
    if (result < 0) {
        write_log("Failed to write to handle");
    }

    return result;
}
