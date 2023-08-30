#include "main.h"

#define VENDOR_ID 0x4444
#define PRODUCT_ID 0x1111
#define TARGET_USAGE_PAGE 0xfacc
#define TARGET_USAGE 0x41

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4000

SharedData sharedData;

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

    // Initialize the mutex and the event
    sharedData.mutex = CreateMutex(NULL, FALSE, NULL);
    if (sharedData.mutex == NULL) {
        fprintf(stderr, "Failed to create mutex.\n");
        return 1;
    }

    sharedData.data_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sharedData.data_ready_event == NULL) {
        fprintf(stderr, "Failed to create event.\n");
        return 1;
    }

    DWORD rawhid_thread_id;
    HANDLE rawhid_thread = CreateThread(NULL, 0, rawhid_device_thread, &device_info, 0, &rawhid_thread_id);
    if (rawhid_thread == NULL) {
        fprintf(stderr, "Error creating hid thread\n");
        return 1;
    }

    DWORD client_thread_id;
    HANDLE client_thread = CreateThread(NULL, 0, tcp_client_thread, &server_info, 0, &client_thread_id);
    if (client_thread == NULL) {
        fprintf(stderr, "Error creating tcp thread\n");
        return 1;
    }
    
    WaitForSingleObject(rawhid_thread, INFINITE);
    WaitForSingleObject(client_thread, INFINITE);

    // Cleanup
    CloseHandle(sharedData.mutex);
    CloseHandle(sharedData.data_ready_event);

    return 0;
}