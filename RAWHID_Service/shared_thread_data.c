#include "shared_thread_data.h"

/**
 * Initialize the mutex and event for shared data.
 *
 * @param sharedData Pointer to the shared data structure.
 * @return 1 if initialization is successful, 0 otherwise.
 */
int initialize_shared_data(shared_thread_data* sharedData) {
    // Create a mutex and store its handle in sharedData
    sharedData->mutex = CreateMutex(NULL, FALSE, NULL);
    if (sharedData->mutex == NULL) {
        write_log(_ERROR, "Shared Data - Failed to create mutex.\n");
        return 0; // Initialization failed
    }

    // Initialize data_ready_to_send_event
    sharedData->data_ready_to_send_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sharedData->data_ready_to_send_event == NULL) {
        write_log(_ERROR, "Shared Data - Failed to create event.\n");
        CloseHandle(sharedData->mutex);  // Clean up the mutex before exiting
        return 0; // Initialization failed
    }

    // Initialize response_received_event
    sharedData->response_received_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sharedData->response_received_event == NULL) {
        write_log(_ERROR, "Shared Data - Failed to create event.\n");
        CloseHandle(sharedData->mutex);  // Clean up the mutex before exiting
        return 0; // Initialization failed
    }

    return 1; // Initialization successful
}

// Helper function to log if a Windows API operation failed
void log_if_failed(BOOL success, const char* operation) {
    if (!success) {
        write_log_format(_ERROR, "Shared Data - Failed to %s. Error: %lu", operation, GetLastError());
    }
}

/**
 * Copies a message to the shared data, designated for TCP transmission.
 *
 * @param sharedData Pointer to the shared data structure.
 * @param message Pointer to the message data.
 * @param size Size of the message in bytes.
 */
void set_message_to_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size) {
    // Log the attempt to acquire a mutex
    write_log_format(_DEBUG, "Shared Data - Pre-mutex acquire for message to TCP. Message size: %lu", size);

    // Acquire the mutex before accessing shared data
    if (WaitForSingleObject(sharedData->mutex, INFINITE) != WAIT_OBJECT_0) {
        write_log(_ERROR, "Shared Data - Failed to acquire mutex for message to TCP");
        return; // Mutex acquire failed, exit function
    }

    write_log(_DEBUG, "Shared Data - Mutex acquired for message to TCP");

    // Copy the message to shared data
    memcpy(sharedData->message_to_tcp, message, size);
    write_log(_DEBUG, "Shared Data - Wrote message for TCP:");
    write_log_byte_array(_DEBUG, message, size);
    // Set the event
    SetEvent(sharedData->data_ready_to_send_event);

    // Release the mutex
    log_if_failed(ReleaseMutex(sharedData->mutex), "Shared Data - Failed release mutex for message to TCP");

    write_log(_DEBUG, "Shared Data - Mutex released for message to TCP");
}

/**
 * Copies a message to the shared data, originating from a TCP connection.
 *
 * @param sharedData Pointer to the shared data structure.
 * @param message Pointer to the message data.
 * @param size Size of the message in bytes.
 */
void set_message_from_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size) {
    // Log the attempt to acquire a mutex
    write_log_format(_DEBUG, "Shared Data - Pre-mutex acquire for message from TCP. Message size: %lu", size);

    // Acquire the mutex before accessing shared data
    if (WaitForSingleObject(sharedData->mutex, INFINITE) != WAIT_OBJECT_0) {
        write_log(_ERROR, "Shared Data - Failed to acquire mutex for message from TCP");
        return; // Mutex acquire failed, exit function
    }

    write_log(_DEBUG, "Shared Data - Mutex acquired for message from TCP");

    // Copy the message to shared data
    memcpy(sharedData->message_from_tcp, message, size);
    write_log(_DEBUG, "Shared Data - Wrote message from TCP:");
    write_log_byte_array(_DEBUG, message, size);

    // Set the event
    SetEvent(sharedData->response_received_event);

    // Release the mutex
    log_if_failed(ReleaseMutex(sharedData->mutex), "Shared Data - Failed release mutex for message from TCP");

    write_log(_DEBUG, "Shared Data - Mutex released for message from TCP");
}

BOOL check_message_to_tcp(shared_thread_data* sharedData, unsigned char* buffer, size_t size) {
    if (WaitForSingleObject(sharedData->data_ready_to_send_event, 0) == WAIT_OBJECT_0) {
        memcpy(buffer, sharedData->message_to_tcp, size);
        ResetEvent(sharedData->data_ready_to_send_event);
        return TRUE;
    }
    return FALSE;
}

BOOL check_message_from_tcp(shared_thread_data* sharedData, unsigned char* buffer, size_t size) {
    if (WaitForSingleObject(sharedData->response_received_event, 0) == WAIT_OBJECT_0) {
        memcpy(buffer, sharedData->message_from_tcp, size);
        ResetEvent(sharedData->response_received_event);
        return TRUE;
    }
    return FALSE;
}

/**
 * Cleans up the shared data by closing handles to the mutex and event.
 *
 * @param sharedData Pointer to the shared data structure.
 */
void cleanup_shared_data(shared_thread_data* sharedData) {
    // Close the mutex handle if it's valid
    if (sharedData->mutex) {
        CloseHandle(sharedData->mutex);
        write_log(_DEBUG, "Shared Data - Mutex closed.");
    }

    // Close the event handle if it's valid
    if (sharedData->data_ready_to_send_event) {
        CloseHandle(sharedData->data_ready_to_send_event);
        write_log(_DEBUG, "Shared Data - Event handle closed.");
    }

    // Close the event handle if it's valid
    if (sharedData->response_received_event) {
        CloseHandle(sharedData->response_received_event);
        write_log(_DEBUG, "Shared Data - Event handle closed.");
    }
}
