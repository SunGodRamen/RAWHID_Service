#include "tcp_client_thread.h"

// Connect to the TCP Server, send the message, log the result
DWORD WINAPI tcp_client_thread(LPVOID server_info) {
    SOCKET clientSocket = init_client(*(tcp_socket_info*)server_info);

    char buffer[MAX_MESSAGE_SIZE];
    while (1){

        WaitForSingleObject(sharedData.data_ready_event, INFINITE);  // Wait for new data
        WaitForSingleObject(sharedData.mutex, INFINITE);

        uint64_t request = 0x4000000000000000;
        send_to_server(clientSocket, (const char*)&request, sizeof(request)); // Send encoded 64-bit request


        uint64_t response_data = 0; // Renamed the variable for clarity
        MessageType response_type;

        int timeoutCounter = 0;
        while (true) {  // Loop until confirmation or timeout
            int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);

            if (response_data != 0) {
                interpret_message(response_data, &response_type);
                if (bytesRead == sizeof(uint64_t) && response_type == CONFIRM_MESSAGE) {
                    break;  // Break out of the loop if confirmation is received
                }

                if (++timeoutCounter >= 10) {  // This will give a total of 10*SOCKET_TIMEOUT_MS time for waiting for the confirmation.
                    write_log("Did not receive a proper confirmation from the server.");
                    break;
                }
            }
        }

        if (response_data) {
            int totalBytesRead = 0;
            int timeoutCounter = 0;

            while (totalBytesRead < sizeof(uint64_t) && timeoutCounter < 10) {
                int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);

                if (totalBytesRead == sizeof(uint64_t)) {
                    break;
                }
                if (bytesRead > 0) {
                    totalBytesRead += bytesRead;
                }
                else {
                    timeoutCounter++;
                }
            }
            sharedData.message_from_tcp = response_data;
            ResetEvent(sharedData.data_ready_event);  // Reset event to non-signaled state
            ReleaseMutex(sharedData.mutex);
        }

    }


    cleanup_client(clientSocket);
    return 0;
}