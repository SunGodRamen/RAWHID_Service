#include "logger.h"

/**
 * Constants for maximum log size and general buffer size for temporary string operations.
 */
#define MAX_LOG_SIZE 512
#define BUFFER_SIZE 4096

 /**
  * Internal variables to keep track of the log file and mutex.
  * Marked as 'static' to limit their scope to this file.
  */
static FILE* logFile = NULL;
static HANDLE logMutex = NULL;

/**
 * Internal utility function to write to the log file.
 *
 * @param message The message string to be logged.
 */
static void write_to_log_file(const char* message);

/**
 * Initialize the logger.
 *
 * @param filePath The path of the file to be used for logging.
 */
void init_logger(char* filePath) {
    errno_t err = fopen_s(&logFile, filePath, "a");
    if (err != 0) {
        perror("Error opening file");
        exit(-1);
    }

    logMutex = CreateMutex(NULL, FALSE, NULL);
    if (logMutex == NULL) {
        fprintf(stderr, "Error: Unable to create mutex.\n");
        fclose(logFile);
        exit(-1);
    }
}

/**
 * Writes a simple log message.
 *
 * @param message The message string to be logged.
 */
void write_log(const char* message) {
    write_to_log_file(message);
}

/**
 * Writes a formatted log message.
 *
 * @param format A format string for the log message.
 * @param ... Variable arguments for the format string.
 */
void write_log_format(const char* format, ...) {
    char buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    write_to_log_file(buffer);
}

/**
 * Logs an unsigned 64-bit integer in decimal format.
 *
 * @param message A message string to prefix the log.
 * @param value The 64-bit unsigned integer to log.
 */
void write_log_uint64_dec(const char* message, uint64_t value) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s: %llu", message, value);

    write_to_log_file(buffer);
}

/**
 * Logs an unsigned 64-bit integer in hexadecimal format.
 *
 * @param message A message string to prefix the log.
 * @param value The 64-bit unsigned integer to log.
 */
void write_log_uint64_hex(const char* message, uint64_t value) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s: 0x%llx", message, value);

    write_to_log_file(buffer);
}

/**
 * Logs an unsigned 64-bit integer in binary format.
 *
 * @param message A message string to prefix the log.
 * @param value The 64-bit unsigned integer to log.
 */
void write_log_uint64_bin(const char* message, uint64_t value) {
    char buffer[BUFFER_SIZE];
    char binaryStr[65];

    for (int i = 63; i >= 0; i--) {
        binaryStr[63 - i] = (value & (1ULL << i)) ? '1' : '0';
    }
    binaryStr[64] = '\0';

    snprintf(buffer, sizeof(buffer), "%s: %s", message, binaryStr);

    write_to_log_file(buffer);
}

/**
 * Internal utility function to write to log file.
 *
 * @param message The message string to be logged.
 */
static void write_to_log_file(const char* message) {
    WaitForSingleObject(logMutex, INFINITE);

    if (fprintf(logFile, "%s\n", message) < 0) {
        fprintf(stderr, "Error: Unable to write to log file.\n");
    }

    fflush(logFile);
    ReleaseMutex(logMutex);
}

/**
 * Close and clean up the logger.
 */
void close_logger() {
    if (logFile) {
        fclose(logFile);
    }
    if (logMutex) {
        CloseHandle(logMutex);
    }
}
