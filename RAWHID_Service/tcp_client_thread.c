#include "tcp_client_thread.h"

// Define constants for maximum number of timeout attempts, confirmation message type, and buffer size
#define MAX_TIMEOUT_COUNTER 10
#define CONFIRMATION_TYPE CONFIRM_MESSAGE
#define BUFFER_SIZE MESSAGE_SIZE_BYTES

/**
 * Thread function for handling TCP client operations.
 * Establishes connection, sends/receives messages, and updates shared data.
 *
 * @param thread_config: Pointer to the configuration structure for this thread
 * @return 0 on success, error code otherwise
 */
DWORD WINAPI tcp_client_thread(LPVOID thread_config) {
    write_log(_INFO, "TCP Client Thread - TCP client thread started.");

    int ret = 0;  // Return code
    SOCKET clientSocket = INVALID_SOCKET;  // Initialize socket to INVALID_SOCKET
    client_thread_config* config = (client_thread_config*)thread_config;  // Cast the void pointer to the expected struct type

    // Check if the required configuration is present
    if (!config || !config->server_config || !config->shared_data) {
        write_log(_ERROR, "TCP Client Thread - Configuration or Server information is NULL.\n");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    // Initialize the client socket
    write_log(_INFO, "TCP Client Thread - Initializing client socket.");
    clientSocket = init_client(config->server_config);
    if (clientSocket == INVALID_SOCKET) {
        write_log(_ERROR, "TCP Client Thread - Failed to initialize client socket.");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    char buffer[BUFFER_SIZE];  // Buffer for incoming/outgoing data
    shared_thread_data* shared_data = config->shared_data;  // Pointer to the shared data

    // Main client operation loop
    write_log(_INFO, "TCP Client Thread - Entering main client operation loop.");
    while (true) {
        write_log(_DEBUG, "TCP Client Thread - Waiting for data ready event.");
        if (WaitForSingleObject(shared_data->data_ready_event, INFINITE) != WAIT_OBJECT_0) {
            write_log(_ERROR, "TCP Client Thread - Failed to wait for data ready event.");
            ret = -1;
            goto cleanup;
        }

        write_log(_DEBUG, "TCP Client Thread - Acquiring mutex.");
        if (WaitForSingleObject(shared_data->mutex, INFINITE) != WAIT_OBJECT_0) {
            write_log(_ERROR, "TCP Client Thread - Failed to acquire mutex.");
            ret = -1;
            goto cleanup;
        }


        // Read the message to send to TCP server from the shared data
        unsigned char request[MESSAGE_SIZE_BITS];  // <--- Changed to unsigned char array
        memcpy(request, shared_data->message_to_tcp, MESSAGE_SIZE_BITS);  // Assuming message_to_tcp is of type unsigned char[]

        // Log the message that will be sent to the server
        write_log(_DEBUG, "TCP Client Thread - Preparing to send data to TCP server.");

        if (send_to_server(clientSocket, (const char*)request, BUFFER_SIZE) == SOCKET_ERROR) {
            write_log(_ERROR, "TCP Client Thread - Failed to send data to the server.");
            ret = -1;
            goto cleanup;
        }

        write_log_format(_DEBUG, "TCP Client Thread - Message sent to server, awaiting response.", (const char*)request);

        unsigned char response_data[MESSAGE_SIZE_BITS];
        MessageType response_type;
        int timeoutCounter = 0;  // Counter for timeout attempts

        // Loop to read and interpret messages from the server
        while (true) {
            int bytesRead = read_message_from_server(clientSocket, (char*)response_data);
            write_log_format(_INFO, "TCP Client Thread - Received %d bytes from server.", bytesRead);
            write_log_byte_array(_DEBUG, response_data, bytesRead);
            if (bytesRead == SOCKET_ERROR) {
                write_log(_ERROR, "TCP Client Thread - An error occurred while reading from the server.");
                ret = -1;  // Update return code to indicate error
                goto cleanup;
            }

            interpret_message(response_data, &response_type);  // Interpret the received message

            // Check if received message is of the confirmation type
            if (bytesRead == BUFFER_SIZE) {
                if (response_type == CONFIRMATION_TYPE) {
                    write_log(_INFO, "TCP Client Thread - Received confirmation from server.");
                    break;  // Break the loop as we got the confirmation
                }
                write_log(_INFO, "TCP Client Thread - Full 64-bit message received from client.");
            }

            // Check if maximum number of timeout attempts reached
            if (++timeoutCounter >= MAX_TIMEOUT_COUNTER) {
                write_log(_ERROR, "TCP Client Thread - Did not receive a proper confirmation from the server.");
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
                write_log_format(_INFO, "TCP Client Thread - Received %d bytes from server.", bytesRead);
                write_log_byte_array(_DEBUG, response_data, bytesRead);
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

            // Log the message received from the server
            write_log(_DEBUG, "TCP Client Thread - Received message from TCP server");
            write_log_byte_array(_DEBUG, response_data, totalBytesRead);


            // Update shared data and release mutex
            write_log(_DEBUG, "TCP Client Thread - Updating shared data.");
            memcpy(shared_data->message_from_tcp, response_data, MESSAGE_SIZE_BITS);

            ResetEvent(shared_data->data_ready_event);
            if (!ReleaseMutex(shared_data->mutex)) {
                write_log(_ERROR, "TCP Client Thread - Failed to release mutex.");
            }
            write_log(_DEBUG, "TCP Client Thread - Mutex released.");
        }
        else {
            if (!ReleaseMutex(shared_data->mutex)) {
                write_log(_ERROR, "TCP Client Thread - Failed to release mutex.");
            }
            write_log(_DEBUG, "TCP Client Thread - Mutex released, no data to update.");
        }
    }

    // Cleanup
cleanup:
    // Close the client socket if it's valid
    write_log(_INFO, "TCP Client Thread - Starting cleanup process.");
    if (clientSocket != INVALID_SOCKET) {
        cleanup_client(clientSocket);
    }

    // Free the configuration structure
    if (config) {
        free(config);
    }
    write_log(_INFO, "TCP Client Thread - TCP client thread terminated.");
    return ret;  // Return the final result code
}
