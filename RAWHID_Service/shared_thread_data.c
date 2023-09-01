#include "shared_thread_data.h"

/**
 * Initialize the mutex and event for shared data.
 *
 * @param sharedData Pointer to the shared data structure.
 * @return 1 if initialization is successful, 0 otherwise.
 */
int initialize_shared_data(shared_thread_data* sharedData) {
    sharedData->mutex = CreateMutex(NULL, FALSE, NULL);
    if (sharedData->mutex == NULL) {
        write_log(_ERROR, "Failed to create mutex.\n");
        return 0;
    }

    sharedData->data_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sharedData->data_ready_event == NULL) {
        write_log(_ERROR, "Failed to create event.\n");
        CloseHandle(sharedData->mutex);  // Cleanup before exiting
        return 0;
    }

    return 1;
}

void log_if_failed(BOOL success, const char* operation) {
    if (!success) {
        write_log_format(_ERROR, "Failed to %s. Error: %lu", operation, GetLastError());
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
    write_log_format(_DEBUG, "Pre-mutex acquire for message to TCP. Message size: %lu", size);

    if (WaitForSingleObject(sharedData->mutex, INFINITE) != WAIT_OBJECT_0) {
        write_log(_ERROR, "Failed to acquire mutex for message to TCP");
        return;
    }

    write_log(_DEBUG, "Mutex acquired for message to TCP");

    memcpy(sharedData->message_to_tcp, message, size);

    log_if_failed(SetEvent(sharedData->data_ready_event), "set event for message to TCP");

    log_if_failed(ReleaseMutex(sharedData->mutex), "release mutex for message to TCP");

    write_log(_DEBUG, "Mutex released for message to TCP");
}

/**
 * Copies a message to the shared data, originating from a TCP connection.
 *
 * @param sharedData Pointer to the shared data structure.
 * @param message Pointer to the message data.
 * @param size Size of the message in bytes.
 */
void set_message_from_tcp(shared_thread_data* sharedData, const unsigned char* message, size_t size) {
    write_log_format(_DEBUG, "Pre-mutex acquire for message from TCP. Message size: %lu", size);

    if (WaitForSingleObject(sharedData->mutex, INFINITE) != WAIT_OBJECT_0) {
        write_log(_ERROR, "Failed to acquire mutex for message from TCP");
        return;
    }

    write_log(_DEBUG, "Mutex acquired for message from TCP");

    memcpy(sharedData->message_from_tcp, message, size);

    log_if_failed(SetEvent(sharedData->data_ready_event), "set event for message from TCP");

    log_if_failed(ReleaseMutex(sharedData->mutex), "release mutex for message from TCP");

    write_log(_DEBUG, "Mutex released for message from TCP");
}

/**
 * Cleans up the shared data by closing handles to the mutex and event.
 *
 * @param sharedData Pointer to the shared data structure.
 */
void cleanup_shared_data(shared_thread_data* sharedData) {
    if (sharedData->mutex) {
        CloseHandle(sharedData->mutex);
    }
    if (sharedData->data_ready_event) {
        CloseHandle(sharedData->data_ready_event);
    }
}
