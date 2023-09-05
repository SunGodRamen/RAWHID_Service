#include "rawhid_thread.h"

/**
 * The thread function that handles communication with the HID device.
 *
 * @param thread_config Pointer to a hid_thread_config struct containing
 *                      the device information and shared data.
 * @return 0 on successful execution, -1 on failure.
 */
DWORD WINAPI rawhid_device_thread(LPVOID thread_config) {
    write_log(_INFO, "RAWHID Thread - Entered rawhid_device_thread.");
    int ret = 0; // Variable to store the return status
    hid_device* handle = NULL; // Handle for the HID device

    // Cast thread_config to its proper type
    hid_thread_config* config = (hid_thread_config*)thread_config;

    // Validate that configuration and device information isn't NULL
    if (!config || !config->device_info || !config->shared_data) {
        write_log(_ERROR, "RAWHID Thread - Configuration or Device information is NULL.\n");
        ret = -1;
        goto cleanup;
    }

    // Log Vendor ID and Product ID
    write_log_format(_INFO, "RAWHID Thread - Initializing HIDAPI for Vendor ID: 0x%x, Product ID: 0x%x", config->device_info->vendor_id, config->device_info->product_id);

    // Initialize the HID API
    if (hid_init()) {
        write_log(_ERROR, "RAWHID Thread - Unable to initialize HIDAPI.\n");
        ret = -1;
        goto cleanup;
    }

    // Get and open the device handle
    handle = get_handle(config->device_info);
    open_usage_path(config->device_info, &handle);
    if (!handle) {
        write_log(_ERROR, "RAWHID Thread - Failed to open the device.\n");
        ret = -1;
        goto cleanup;
    }

    write_log(_INFO, "RAWHID Thread - Device opened successfully.");
    
    // Set the device to non-blocking mode
    hid_set_nonblocking(handle, true);

    // Get the shared data
    shared_thread_data* shared_data = config->shared_data;
    unsigned char message_from_hid[MESSAGE_SIZE_BYTES];
    unsigned char message_from_tcp[MESSAGE_SIZE_BYTES];

    // Main loop for reading from the device
    while (true) {
        int bytes_read = hid_read(handle, message_from_hid, sizeof(message_from_hid));
        if (bytes_read < 0) {
            write_log_format(_ERROR, "RAWHID Thread - Failed to read from device: %s", hid_error(handle));
            ret = -1;
            goto cleanup;
        }

        // If read is successful
        if (bytes_read > 0) {
            write_log_format(_INFO, "RAWHID Thread - Number of bytes read: %d", bytes_read);
            // Log the byte array using your new function
            write_log_byte_array(_DEBUG, message_from_hid, MESSAGE_SIZE_BYTES);

            // Set the message to be sent over TCP
            set_message_to_tcp(shared_data, message_from_hid, MESSAGE_SIZE_BYTES);
        }

        // Wait for the response_received_event to be set, signaling a new message from TCP client
        if (check_message_from_tcp( shared_data, message_from_tcp, MESSAGE_SIZE_BYTES)) {

            // Now you can send this message to HID device
            if (write_to_handle(handle, message_from_tcp, MESSAGE_SIZE_BYTES) < 0) {
                write_log(_ERROR, "RAWHID Thread - Failed to send message to device");
            }
        }

    }

cleanup: // Cleanup label for resource freeing and exit
    if (handle) {
        hid_close(handle);
    }
    hid_exit();
    if (config) {
        free(config);
    }

    write_log(_INFO, "RAWHID Thread - Exiting rawhid_device_thread.");
    return ret;
}

