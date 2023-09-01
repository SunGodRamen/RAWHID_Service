#include "rawhid_thread.h"

// Function prototype for sending a ping message
void send_ping(hid_device* handle);

/**
 * The thread function that handles communication with the HID device.
 *
 * @param thread_config Pointer to a hid_thread_config struct containing
 *                      the device information and shared data.
 * @return 0 on successful execution, -1 on failure.
 */
DWORD WINAPI rawhid_device_thread(LPVOID thread_config) {
    int ret = 0; // Use to collect the return status
    hid_device* handle = NULL;
    unsigned char message[MESSAGE_SIZE_BITS] = { 0 };

    hid_thread_config* config = (hid_thread_config*)thread_config;
    if (!config || !config->device_info || !config->shared_data) {
        write_log(_ERROR, "Configuration or Device information is NULL.\n");
        ret = -1;
        goto cleanup;
    }

    write_log_format(_INFO, "Vendor ID: 0x%x, Product ID: 0x%x", config->device_info->vendor_id, config->device_info->product_id);

    if (hid_init()) {
        write_log(_ERROR, "Unable to initialize HIDAPI.\n");
        ret = -1;
        goto cleanup;
    }

    handle = get_handle(config->device_info);
    open_usage_path(config->device_info, &handle);
    if (!handle) {
        write_log(_ERROR, "Failed to open the device.\n");
        ret = -1;
        goto cleanup;
    }

    hid_set_nonblocking(handle, true);
    shared_thread_data* shared_data = config->shared_data;

    while (true) {
        int read_status = hid_read(handle, message, sizeof(message));
        if (read_status < 0) {
            write_log_format(_ERROR, "Failed to read from device: %s", hid_error(handle));
            ret = -1;
            goto cleanup;
        }

        if (read_status > 0) {
            write_log(_DEBUG, "hid read", message);
            set_message_to_tcp(shared_data, message, sizeof(message));
            send_ping(handle);
        }
    }

cleanup:
    if (handle) {
        hid_close(handle);
    }
    hid_exit();
    if (config) {
        free(config);
    }
    return ret;
}

/**
 * Sends a ping message to a HID device.
 *
 * @param handle The handle to the HID device.
 */
void send_ping(hid_device* handle) {
    // Validate the device handle
    if (!handle) {
        write_log(_ERROR, "Handle is NULL, cannot send ping");
        return;
    }

    // Prepare the ping message
    unsigned char ping[] = { 0x00, 0x01 };
    // Attempt to send the ping message
    if (write_to_handle(handle, ping, sizeof(ping)) < 0) {
        write_log_format(_ERROR, "Failed to send ping: %s", hid_error(handle));
    }
}
