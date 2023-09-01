#include "tcp_client.h"

/**
 * Reads a 64-bit (8 bytes) message from the server.
 *
 * @param serverSocket The server socket to read the message from.
 * @param buffer The buffer to store the message.
 * @return The number of bytes read, or -1 on error.
 */
int read_message_from_server(SOCKET serverSocket, char* buffer) {
    if (!buffer) {
        write_log(_ERROR, "Buffer is NULL");
        return -1;
    }

    int totalBytesRead = 0;
    int bytesRead = 0;

    while (totalBytesRead < MESSAGE_SIZE_BYTES) {
        bytesRead = recv(serverSocket, buffer + totalBytesRead, MESSAGE_SIZE_BYTES - totalBytesRead, 0);

        if (bytesRead == SOCKET_ERROR && WSAGetLastError() != 10053) {
            write_log_format(_ERROR, "TCP: Error occurred while reading from socket. Error Code: %d", WSAGetLastError());
            return -1;
        }

        if (bytesRead == 0) {
            write_log(_ERROR, "TCP: Server disconnected before sending full message.");
            return totalBytesRead;
        }

        totalBytesRead += bytesRead;
    }

    return totalBytesRead;
}

/**
 * Initializes the TCP client and connects to the server.
 *
 * @param server_info Pointer to a tcp_socket_info struct containing server details.
 * @return A valid socket to the server, or INVALID_SOCKET on failure.
 */
SOCKET init_client(tcp_socket_info* server_info) {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        write_log_format(_ERROR, "TCP: Failed to initialize WinSock. Error Code: %d", WSAGetLastError());
        return INVALID_SOCKET;
    }

    if (!server_info) {
        write_log(_ERROR, "Server information is NULL");
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        write_log_format(_ERROR, "TCP: Failed to create socket. Error Code: %d; Server IP: %s, Port: %d",
            WSAGetLastError(), server_info->ip, server_info->port);
        WSACleanup();
        return INVALID_SOCKET;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;

    if (inet_pton(AF_INET, server_info->ip, &(serverAddr.sin_addr)) <= 0) {
        write_log_format(_ERROR, "TCP: Invalid IP address or error in inet_pton. Error Code: %d; Server IP: %s, Port: %d",
            WSAGetLastError(), server_info->ip, server_info->port);
        cleanup_client(clientSocket);
        return INVALID_SOCKET;
    }

    serverAddr.sin_port = htons(server_info->port);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        write_log_format(_ERROR, "TCP: Connect failed. Error Code: %d; Server IP: %s, Port: %d",
            WSAGetLastError(), server_info->ip, server_info->port);
        cleanup_client(clientSocket);
        return INVALID_SOCKET;
    }

    return clientSocket;
}

/**
 * Sends data to the server.
 *
 * @param serverSocket The server socket to send the data to.
 * @param data Pointer to the data to be sent.
 * @param dataLength The length of the data in bytes.
 */
int send_to_server(SOCKET serverSocket, const char* data, int dataLength) {
    if (!data || dataLength <= 0) {
        write_log(_ERROR, "Invalid data to send");
        return -1; // Fixed a typo, added a semicolon
    }

    if (send(serverSocket, data, dataLength, 0) == SOCKET_ERROR) {
        write_log_format(_ERROR, "Failed to send data. Error Code: %d", WSAGetLastError());
        return -1;
    }

    write_log_format(_DEBUG, "Sending to tcp server: %s", data);

    return 0;
}

/**
 * Cleans up the client by closing the socket and cleaning up WinSock resources.
 *
 * @param serverSocket The server socket to close.
 */
void cleanup_client(SOCKET serverSocket) {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
    WSACleanup();
}
