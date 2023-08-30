#include "rawhid_thread.h"

DWORD WINAPI rawhid_device_thread(LPVOID device_info) {

    if (hid_init()) {
        fprintf(stderr, "Unable to initialize HIDAPI.\n");
        return -1;
    }

    hid_device* handle = get_handle(&device_info);
    open_usage_path(&device_info, &handle);

    if (handle) {
        unsigned char message[MAX_MESSAGE_SIZE];
        hid_set_nonblocking(handle, true);

        while (true) {
            int res = hid_read(handle, message, sizeof(message));

            if (res) {
                WaitForSingleObject(sharedData.mutex, INFINITE);
                memcpy(sharedData.message_to_tcp, message, MAX_MESSAGE_SIZE);
                SetEvent(sharedData.data_ready_event);
                ReleaseMutex(sharedData.mutex);
            }
        }

        hid_close(handle);
    }
    else {
        fprintf(stderr, "Failed to open the device.\n");
    }

    hid_exit();

    return 0;
}
