#include "rawhid_thread.h"

#define BUFFER_SIZE MESSAGE_SIZE_BITS

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
    write_log(_INFO, "RAWHID Thread - Entered rawhid_device_thread.");
    int ret = 0; // Variable to store the return status
    hid_device* handle = NULL; // Handle for the HID device
    unsigned char message[BUFFER_SIZE]; // Message buffer

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

    // Main loop for reading from the device
    while (true) {
        int read_status = hid_read(handle, message, sizeof(message));
        if (read_status < 0) {
            write_log_format(_ERROR, "RAWHID Thread - Failed to read from device: %s", hid_error(handle));
            ret = -1;
            goto cleanup;
        }

        // If read is successful
        if (read_status > 0) {
            // Log the byte array using your new function
            write_log_byte_array(_DEBUG, message, read_status);

            // Set the message to be sent over TCP
            set_message_to_tcp(shared_data, message, sizeof(message));

            // Send a ping message
            send_ping(handle);
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

/**
 * Sends a ping message to a HID device.
 *
 * @param handle The handle to the HID device.
 */
void send_ping(hid_device* handle) {
    write_log(_INFO, "Preparing to send ping.");
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
    else {
        write_log(_INFO, "Ping sent successfully.");
    }
}
