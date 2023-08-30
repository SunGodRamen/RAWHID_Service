#include "rawhid.h"

hid_device* get_handle(hid_usage_info* usage_info) {
    hid_device* handle = hid_open(usage_info->vendor_id, usage_info->product_id, NULL);
    return handle;
}

void open_usage_path(hid_usage_info* usage_info, hid_device** handle) {
    if (*handle) {
        struct hid_device_info* enum_device_info = hid_enumerate(&usage_info->vendor_id, &usage_info->product_id);
        // Loop through device info list
        for (; enum_device_info != NULL; enum_device_info = enum_device_info->next) {
            if (enum_device_info->usage_page == usage_info->usage_page && enum_device_info->usage == usage_info->usage) {
                *handle = hid_open_path(enum_device_info->path);
                if (*handle) {
                    write_log("  Successfully opened device");
                    break;
                }
            }
        }
        hid_free_enumeration(enum_device_info);
    }
    else {
        write_log("no handle to open");
    }
}

// Function to write to the handle
int write_to_handle(hid_device* handle, unsigned char* message, size_t size) {
    return hid_write(handle, message, size);
}

