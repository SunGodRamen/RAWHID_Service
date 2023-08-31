#include "tcp_client_thread.h"

#define MAX_TIMEOUT_COUNTER 10  // Max timeout attempts
#define CONFIRMATION_TYPE CONFIRM_MESSAGE  // Confirmation message type
#define BUFFER_SIZE sizeof(uint64_t)  // Size of buffer in bytes (64-bit)

// Connect to the TCP Server, send the message, log the result
DWORD WINAPI tcp_client_thread(LPVOID server_info) {
    SOCKET clientSocket = init_client(*(tcp_socket_info*)server_info);

    char buffer[BUFFER_SIZE];

    while (true) {
        // Wait for new data
        WaitForSingleObject(sharedData.data_ready_event, INFINITE);
        WaitForSingleObject(sharedData.mutex, INFINITE);

        uint64_t request = sharedData.message_to_tcp;

        // Send encoded 64-bit request
        send_to_server(clientSocket, (const char*)&request, BUFFER_SIZE);

        uint64_t response_data = 0;
        MessageType response_type;
        int timeoutCounter = 0;

        // Loop until confirmation or timeout
        while (true) {
            int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);

            if (bytesRead == SOCKET_ERROR) {
                write_log("An error occurred while reading from the server.");
                break;
            }

            // Interpret and handle the response
            interpret_message(response_data, &response_type);

            if (bytesRead == BUFFER_SIZE && response_type == CONFIRMATION_TYPE) {
                break;
            }

            if (++timeoutCounter >= MAX_TIMEOUT_COUNTER) {
                write_log("Did not receive a proper confirmation from the server.");
                break;
            }
        }

        if (response_data) {
            int totalBytesRead = 0;
            int timeoutCounter = 0;

            while (totalBytesRead < BUFFER_SIZE && timeoutCounter < MAX_TIMEOUT_COUNTER) {
                int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);

                if (bytesRead > 0) {
                    totalBytesRead += bytesRead;
                }
                else {
                    timeoutCounter++;
                }

                if (totalBytesRead == BUFFER_SIZE) {
                    break;
                }
            }

            sharedData.message_from_tcp = response_data;

            // Reset event to non-signaled state
            ResetEvent(sharedData.data_ready_event);

            // Release the mutex
            ReleaseMutex(sharedData.mutex);
        }
    }

    cleanup_client(clientSocket);
    return 0;
}
