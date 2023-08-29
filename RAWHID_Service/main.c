#include "main.h"

#define VENDOR_ID 0x4444
#define PRODUCT_ID 0x1111
#define TARGET_USAGE_PAGE 0xfacc
#define TARGET_USAGE 0x41

#define PING_INTERVAL 1

DWORD WINAPI ThreadFunc(void* data) {
    hid_device* handle = (hid_device*)data;
    unsigned char message[64];
    unsigned char pong[] = { 0x02 };
    int successCount = 0;
    time_t lastPingTime = 0;
    bool lastPingPonged = true;

    hid_set_nonblocking(handle, 1);

    while (true) {
        if (time(NULL) - lastPingTime >= PING_INTERVAL) {
            if (!lastPingPonged) {
                printf("no response\n");
            }
            lastPingPonged = false;
            send_ping(handle);
            lastPingTime = time(NULL);
        }

        int res = hid_read(handle, message, sizeof(message));
        if (process_message(handle, message, res, pong)) {
            lastPingPonged = true; // Assume process_message returns true if pong is successful
        }
    }
    return 0;
}

int main() {
    if (hid_init()) {
        fprintf(stderr, "Unable to initialize HIDAPI.\n");
        return -1;
    }

    hid_device* handle = get_handle(VENDOR_ID, PRODUCT_ID);
    open_usage_path(VENDOR_ID, PRODUCT_ID, TARGET_USAGE_PAGE, TARGET_USAGE, &handle);

    if (handle) {
        DWORD ThreadId;
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, handle, 0, &ThreadId);

        if (hThread == NULL) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }

        // Wait for the thread to complete (optional)
        WaitForSingleObject(hThread, INFINITE);

        hid_close(handle);
    }
    else {
        fprintf(stderr, "Failed to open the device.\n");
    }

    hid_exit();
    return 0;
}