#include "rawhid_thread.h"

#ifndef MESSAGE_SIZE_BITS
#define MESSAGE_SIZE_BITS 64
#endif

DWORD WINAPI rawhid_device_thread(LPVOID device_info) {

    // Initialize HID API
    if (hid_init()) {
        fprintf(stderr, "Unable to initialize HIDAPI.\n");
        return -1;
    }

    // Validate and get a device handle
    if (!device_info) {
        fprintf(stderr, "Device information is NULL.\n");
        hid_exit();
        return -1;
    }

    hid_device* handle = get_handle(device_info);
    open_usage_path(device_info, &handle);

    // Check if handle is valid
    if (!handle) {
        fprintf(stderr, "Failed to open the device.\n");
        hid_exit();
        return -1;
    }

    // Core logic
    unsigned char message[MESSAGE_SIZE_BITS];
    hid_set_nonblocking(handle, true);

    while (true) {
        int read_status = hid_read(handle, message, sizeof(message));
        if (read_status > 0) {
            // Acquire the mutex and update the shared data
            WaitForSingleObject(sharedData.mutex, INFINITE);
            memcpy(sharedData.message_to_tcp, message, MESSAGE_SIZE_BITS);
            SetEvent(sharedData.data_ready_event);
            ReleaseMutex(sharedData.mutex);
        }
        // Send a ping
        send_ping(handle);
    }

    // Close and exit
    hid_close(handle);
    hid_exit();
    return 0;
}

// Function to send a ping message
void send_ping(hid_device* handle) {
    if (!handle) {
        write_log("Handle is NULL, cannot send ping");
        return;
    }

    unsigned char ping[] = { 0x00, 0x01 };
    if (write_to_handle(handle, ping, sizeof(ping)) < 0) {
        write_log_format("Failed to send ping: %s", hid_error(handle));
    }
}
