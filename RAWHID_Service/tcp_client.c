#include "tcp_client.h"

/**
 * Reads a 64-bit (8 bytes) message from the server.
 *
 * @param serverSocket The server socket to read the message from.
 * @param buffer The buffer to store the message.
 * @return The number of bytes read, or -1 on error.
 */
int read_message_from_server(SOCKET serverSocket, char* buffer) {
    int totalBytesRead = 0;
    int bytesRead = 0;
    int bytesToRead = 8; // 64 bits

    while (totalBytesRead < bytesToRead) {
        bytesRead = recv(serverSocket, buffer + totalBytesRead, bytesToRead - totalBytesRead, 0);
        if (bytesRead == SOCKET_ERROR && WSAGetLastError() != 10053) {
            // Error handling code remains the same as before
            return -1;
        }
        else if (bytesRead == 0) {
            // Server disconnected before sending full message.
            write_log("TCP: Server disconnected before sending full message.");
            return totalBytesRead;
        }
        totalBytesRead += bytesRead;
    }

    return totalBytesRead;
}


SOCKET init_client(tcp_socket_info* server_info) {
    WSADATA wsaData;

    // Initialize WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        write_log_format("TCP: Failed to initialize WinSock. Error Code: %d", WSAGetLastError());
        exit(1);
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        write_log_format("TCP: Failed to create socket. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;

    // Convert IP string to a network byte order using inet_pton
    if (inet_pton(AF_INET, server_info->ip, &(serverAddr.sin_addr)) <= 0) {
        write_log_format("TCP: Invalid IP address or error in inet_pton. Error Code: %d", WSAGetLastError());
        closesocket(clientSocket);
        exit(1);
    }

    serverAddr.sin_port = htons(server_info->port);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        write_log("TCP: Connect failed. Error Code: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        exit(1);
    }

    return clientSocket;
}

void send_to_server(SOCKET serverSocket, const char* data, int dataLength) {
    send(serverSocket, data, dataLength, 0);
}

void cleanup_client(SOCKET serverSocket) {
    if (serverSocket) {
        closesocket(serverSocket);
    }
    WSACleanup();
}
