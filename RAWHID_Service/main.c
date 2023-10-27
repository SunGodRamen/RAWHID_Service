#include "config.h"
#include "tcp_client_thread.h"
#include "rawhid_thread.h"
#include "shared_thread_data.h"
#include "logger.h"
#include <windows.h>

#define LOG_LEVEL LOGLEVEL_INFO

int create_threads(HANDLE* rawhid_thread, HANDLE* client_thread, hid_usage_info* device_info, tcp_socket_info* server_info, shared_thread_data* shared_data);

int main() {

    // Initialization code
    init_logger(LOG_FILE);
    set_log_level(LOG_LEVEL);

    // Logging application start
    write_log(LOGLEVEL_INFO, "Main - Application started");

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

    // Initialize shared data
    shared_thread_data shared_data;
    if (!initialize_shared_data(&shared_data)) {
        write_log(LOGLEVEL_ERROR, "Main - Failed to initialize shared data");
        return 1;
    }
    write_log(LOGLEVEL_INFO, "Main - Shared data initialized");

    // Create threads
    HANDLE rawhid_thread, client_thread;
    if (!create_threads(&rawhid_thread, &client_thread, &device_info, &server_info, &shared_data)) {
        write_log(LOGLEVEL_ERROR, "Main - Failed to create threads");
        return 1;
    }
    write_log(LOGLEVEL_INFO, "Main - Threads created");

    // Wait for threads to complete
    WaitForSingleObject(rawhid_thread, INFINITE);
    WaitForSingleObject(client_thread, INFINITE);

    // Cleanup
    cleanup_shared_data(&shared_data);
    write_log(LOGLEVEL_INFO, "Main - Cleanup completed");

    // Close logger
    close_logger();
    write_log(LOGLEVEL_INFO, "Main - Application exiting successfully");  // Note: This won't be written to the file as the logger is closed, but it's useful for debugging

    return 0;
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
int create_threads(HANDLE* rawhid_thread, HANDLE* client_thread, hid_usage_info* device_info, tcp_socket_info* server_info, shared_thread_data* shared_data) {
    
    hid_thread_config* hid_thread_config_ptr = (hid_thread_config*)malloc(sizeof(hid_thread_config));
    if (hid_thread_config_ptr == NULL) {
        write_log(LOGLEVEL_ERROR, "Main - Error allocating memory for hid_thread_config\n");
        return 0;
    }
    hid_thread_config_ptr->device_info = device_info;
    hid_thread_config_ptr->shared_data = shared_data;

    client_thread_config* client_thread_config_ptr = (client_thread_config*)malloc(sizeof(client_thread_config));
    if (client_thread_config_ptr == NULL) {
        write_log(LOGLEVEL_ERROR, "Main - Error allocating memory for client_thread_config\n");
        free(hid_thread_config_ptr);
        return 0;
    }
    client_thread_config_ptr->server_config = server_info;
    client_thread_config_ptr->shared_data = shared_data;
    
    DWORD rawhid_thread_id, client_thread_id;

    *rawhid_thread = CreateThread(NULL, 0, rawhid_device_thread, hid_thread_config_ptr, 0, &rawhid_thread_id);
    if (*rawhid_thread == NULL) {
        write_log(LOGLEVEL_ERROR, "Main - Error creating hid thread\n");
        free(hid_thread_config_ptr);
        free(client_thread_config_ptr);
        return 0;
    }

    *client_thread = CreateThread(NULL, 0, tcp_client_thread, client_thread_config_ptr, 0, &client_thread_id);
    if (*client_thread == NULL) {
        write_log(LOGLEVEL_ERROR, "Main - Error creating tcp thread\n");
        CloseHandle(*rawhid_thread);  // Cleanup before exiting
        free(hid_thread_config_ptr);
        free(client_thread_config_ptr);
        return 0;
    }

    return 1;
}
