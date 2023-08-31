#include "config.h"
#include "tcp_client.h"
#include "tcp_client_thread.h"
#include "rawhid.h"
#include "rawhid_thread.h"
#include "interthread_comm.h"
#include "logger.h"
#include <windows.h>
#include <stdbool.h>

SharedData sharedData;

int initialize_shared_data();
int create_threads(HANDLE* rawhid_thread, HANDLE* client_thread, hid_usage_info* device_info, tcp_socket_info* server_info);

int main() {
    hid_usage_info device_info = {
        .vendor_id = VENDOR_ID,
        .product_id = PRODUCT_ID,
        .usage_page = TARGET_USAGE_PAGE,
        .usage = TARGET_USAGE
    };

    tcp_socket_info server_info = {
        .ip = SERVER_IP,
        .port = SERVER_PORT
    };

    // Initialize logging
    init_logger(LOG_FILE);

    // Initialize shared data
    if (!initialize_shared_data()) {
        return 1;
    }

    // Create threads
    HANDLE rawhid_thread, client_thread;
    if (!create_threads(&rawhid_thread, &client_thread, &device_info, &server_info)) {
        return 1;
    }

    // Wait for threads to complete
    WaitForSingleObject(rawhid_thread, INFINITE);
    WaitForSingleObject(client_thread, INFINITE);

    // Cleanup
    CloseHandle(sharedData.mutex);
    CloseHandle(sharedData.data_ready_event);

    // Close logger
    close_logger();

    return 0;
}

/**
 * Initialize the mutex and event for shared data
 *
 * @return 1 if successful, 0 otherwise
 */
int initialize_shared_data() {
    sharedData.mutex = CreateMutex(NULL, FALSE, NULL);
    if (sharedData.mutex == NULL) {
        fprintf(stderr, "Failed to create mutex.\n");
        return 0;
    }

    sharedData.data_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sharedData.data_ready_event == NULL) {
        fprintf(stderr, "Failed to create event.\n");
        CloseHandle(sharedData.mutex);  // Cleanup before exiting
        return 0;
    }

    return 1;
}

/**
 * Create rawhid and client threads
 *
 * @param rawhid_thread Pointer to handle for rawhid thread
 * @param client_thread Pointer to handle for client thread
 * @param device_info Pointer to hid_usage_info for rawhid thread
 * @param server_info Pointer to tcp_socket_info for client thread
 * @return 1 if successful, 0 otherwise
 */
int create_threads(HANDLE* rawhid_thread, HANDLE* client_thread, hid_usage_info* device_info, tcp_socket_info* server_info) {
    DWORD rawhid_thread_id, client_thread_id;

    *rawhid_thread = CreateThread(NULL, 0, rawhid_device_thread, device_info, 0, &rawhid_thread_id);
    if (*rawhid_thread == NULL) {
        fprintf(stderr, "Error creating hid thread\n");
        return 0;
    }

    *client_thread = CreateThread(NULL, 0, tcp_client_thread, server_info, 0, &client_thread_id);
    if (*client_thread == NULL) {
        fprintf(stderr, "Error creating tcp thread\n");
        CloseHandle(*rawhid_thread);  // Cleanup before exiting
        return 0;
    }

    return 1;
}
