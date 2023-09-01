#include "tcp_client_thread.h"

// Define constants for maximum number of timeout attempts, confirmation message type, and buffer size
#define MAX_TIMEOUT_COUNTER 10  
#define CONFIRMATION_TYPE CONFIRM_MESSAGE  
#define BUFFER_SIZE sizeof(uint64_t)  

/**
 * Thread function for handling TCP client operations.
 * Establishes connection, sends/receives messages, and updates shared data.
 *
 * @param thread_config: Pointer to the configuration structure for this thread
 * @return 0 on success, error code otherwise
 */
DWORD WINAPI tcp_client_thread(LPVOID thread_config) {
    int ret = 0;  // Return code
    SOCKET clientSocket = INVALID_SOCKET;  // Initialize socket to INVALID_SOCKET
    client_thread_config* config = (client_thread_config*)thread_config;  // Cast the void pointer to the expected struct type

    // Check if the required configuration is present
    if (!config || !config->server_config || !config->shared_data) {
        write_log(_ERROR, "Configuration or Server information is NULL.\n");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    // Initialize the client socket
    clientSocket = init_client(config->server_config);
    if (clientSocket == INVALID_SOCKET) {
        write_log(_ERROR, "Failed to initialize client socket.");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    char buffer[BUFFER_SIZE];  // Buffer for incoming/outgoing data
    shared_thread_data* shared_data = config->shared_data;  // Pointer to the shared data

    // Main client operation loop
    while (true) {
        write_log(_DEBUG, "Waiting for data ready event.");
        if (WaitForSingleObject(shared_data->data_ready_event, INFINITE) != WAIT_OBJECT_0) {
            write_log(_ERROR, "Failed to wait for data ready event.");
            ret = -1;
            goto cleanup;
        }

        write_log(_DEBUG, "Acquiring mutex.");
        if (WaitForSingleObject(shared_data->mutex, INFINITE) != WAIT_OBJECT_0) {
            write_log(_ERROR, "Failed to acquire mutex.");
            ret = -1;
            goto cleanup;
        }


        // Read the message to send to TCP server from the shared data
        uint64_t request = shared_data->message_to_tcp;
        if (send_to_server(clientSocket, (const char*)&request, BUFFER_SIZE) == SOCKET_ERROR) {
            write_log(_ERROR, "Failed to send data to the server.");
            ret = -1;  // Update return code to indicate error
            goto cleanup;
        }

        write_log_format(_DEBUG, "Message sent to server, awaiting response.", (const char*)request);

        uint64_t response_data = 0;
        MessageType response_type;
        int timeoutCounter = 0;  // Counter for timeout attempts

        // Loop to read and interpret messages from the server
        while (true) {
            int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);
            if (bytesRead == SOCKET_ERROR) {
                write_log(_ERROR, "An error occurred while reading from the server.");
                ret = -1;  // Update return code to indicate error
                goto cleanup;
            }

            interpret_message(response_data, &response_type);  // Interpret the received message

            // Check if received message is of the confirmation type
            if (bytesRead == BUFFER_SIZE && response_type == CONFIRMATION_TYPE) {
                break;  // Break the loop as we got the confirmation
            }

            // Check if maximum number of timeout attempts reached
            if (++timeoutCounter >= MAX_TIMEOUT_COUNTER) {
                write_log(_ERROR, "Did not receive a proper confirmation from the server.");
                ret = -1;  // Update return code to indicate error
                goto cleanup;
            }
        }

        // Continue with further processing if response_data is not zero
        if (response_data) {
            int totalBytesRead = 0;  // Total bytes read from the server
            int timeoutCounter = 0;  // Counter for timeout attempts

            // Loop to receive remaining bytes from server
            while (totalBytesRead < BUFFER_SIZE && timeoutCounter < MAX_TIMEOUT_COUNTER) {
                int bytesRead = read_message_from_server(clientSocket, (char*)&response_data);
                if (bytesRead > 0) {
                    totalBytesRead += bytesRead;
                }
                else {
                    timeoutCounter++;  // Increment timeout counter on no data
                }

                // Check if the buffer is full
                if (totalBytesRead == BUFFER_SIZE) {
                    break;  // Exit loop
                }
            }

            // Update shared data and release mutex
            shared_data->message_from_tcp = response_data;
            ResetEvent(shared_data->data_ready_event);
            if (!ReleaseMutex(shared_data->mutex)) {
                write_log(_ERROR, "Failed to release mutex.");
            }
            write_log(_DEBUG, "Mutex released.");
        }
        else {
            if (!ReleaseMutex(shared_data->mutex)) {
                write_log(_ERROR, "Failed to release mutex.");
            }
            write_log(_DEBUG, "Mutex released, no data to update.");
        }
    }

    // Cleanup
cleanup:
    // Close the client socket if it's valid
    if (clientSocket != INVALID_SOCKET) {
        cleanup_client(clientSocket);
    }

    // Free the configuration structure
    if (config) {
        free(config);
    }

    return ret;  // Return the final result code
}
