#include "tcp_client_thread.h"

// Define constants for maximum number of timeout attempts, confirmation message type, and buffer size
#define MAX_TIMEOUT_COUNTER 10

/**
 * Thread function for handling TCP client operations.
 * Establishes connection, sends/receives messages, and updates shared data.
 *
 * @param thread_config: Pointer to the configuration structure for this thread
 * @return 0 on success, error code otherwise
 */
DWORD WINAPI tcp_client_thread(LPVOID thread_config) {
    write_log(LOGLEVEL_INFO, "TCP Client Thread - TCP client thread started.");

    int ret = 0;  // Return code
    SOCKET clientSocket = INVALID_SOCKET;  // Initialize socket to INVALID_SOCKET
    client_thread_config* config = (client_thread_config*)thread_config;  // Cast the void pointer to the expected struct type

    // Check if the required configuration is present
    if (!config || !config->server_config || !config->shared_data) {
        write_log(LOGLEVEL_ERROR, "TCP Client Thread - Configuration or Server information is NULL.\n");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    // Initialize the client socket
    write_log(LOGLEVEL_INFO, "TCP Client Thread - Initializing client socket.");
    clientSocket = init_client(config->server_config);
    if (clientSocket == INVALID_SOCKET) {
        write_log(LOGLEVEL_ERROR, "TCP Client Thread - Failed to initialize client socket.");
        ret = -1;  // Update return code to indicate error
        goto cleanup;
    }

    char buffer[MESSAGE_SIZE_BYTES];  // Buffer for incoming/outgoing data
    shared_thread_data* shared_data = config->shared_data;  // Pointer to the shared data

    // Main client operation loop
    write_log(LOGLEVEL_INFO, "TCP Client Thread - Entering main client operation loop.");
    while (true) {
        // Read the message to send to TCP server from the shared data
        unsigned char request_from_hid[MESSAGE_SIZE_BYTES];
        if (check_message_to_tcp(shared_data, request_from_hid, MESSAGE_SIZE_BYTES)) {

            // Log the message that will be sent to the server
            write_log(LOGLEVEL_DEBUG, "TCP Client Thread - Preparing to send data to TCP server.");

            if (send_to_server(clientSocket, (const char*)request_from_hid, MESSAGE_SIZE_BYTES) == SOCKET_ERROR) {
                write_log(LOGLEVEL_ERROR, "TCP Client Thread - Failed to send data to the server.");
                ret = -1;
                goto cleanup;
            }

            write_log(LOGLEVEL_DEBUG, "TCP Client Thread - Message sent to server, awaiting response.");

            unsigned char confirmation_message[MESSAGE_SIZE_BYTES];
            MessageType message_type;
            int timeoutCounter = 0;  // Counter for timeout attempts

            // Read the confirmation message from the server
            int bytesRead = read_message_from_server(clientSocket, (char*)confirmation_message);
            write_log_format(LOGLEVEL_INFO, "TCP Client Thread - Received %d/%d bytes of confirmation from server.", bytesRead, MESSAGE_SIZE_BYTES);
            write_log_byte_array(LOGLEVEL_DEBUG, confirmation_message, bytesRead);

            if (bytesRead == SOCKET_ERROR) {
                write_log(LOGLEVEL_ERROR, "TCP Client Thread - An error occurred while reading confirmation from the server.");
                ret = -1;  // Update return code to indicate error
                goto cleanup;
            }

            // Interpret the received message
            interpret_message(&confirmation_message, &message_type);
            write_log_format(LOGLEVEL_INFO, "TCP Client Thread - Interpreted message type: %d, expected confirm type: %d", message_type, CONFIRM_MESSAGE);

            // Continue with further processing if the message type is CONFIRMATION_TYPE
            if (message_type == CONFIRM_MESSAGE) {
                unsigned char response[MESSAGE_SIZE_BYTES];

                int bytesRead = read_message_from_server(clientSocket, (char*)&response);
                if (bytesRead != SOCKET_ERROR) {

                    // Log the message received from the server
                    write_log(LOGLEVEL_DEBUG, "TCP Client Thread - Received response from TCP server");
                    write_log_byte_array(LOGLEVEL_DEBUG, response, bytesRead);

                    // Update the shared data with the response from the server
                    set_message_from_tcp(shared_data, response, MESSAGE_SIZE_BYTES);
                }
				else {
					write_log(LOGLEVEL_ERROR, "TCP Client Thread - An error occurred while reading response from the server.");
					ret = -1;  // Update return code to indicate error
					goto cleanup;
				}
            }
            else {
                write_log(LOGLEVEL_DEBUG, "TCP Client Thread - unexpected response.");
            }
        }
    }

    // Cleanup
cleanup:
    // Close the client socket if it's valid
    write_log(LOGLEVEL_INFO, "TCP Client Thread - Starting cleanup process.");
    if (clientSocket != INVALID_SOCKET) {
        cleanup_client(clientSocket);
    }

    // Free the configuration structure
    if (config) {
        free(config);
    }
    write_log(LOGLEVEL_INFO, "TCP Client Thread - TCP client thread terminated.");
    return ret;  // Return the final result code
}
