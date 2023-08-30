#include "logger.h"

FILE* logFile = NULL;
HANDLE logMutex = NULL;

void init_logger(char* filePath) {
    errno_t err = fopen_s(&logFile, filePath, "a");
    if (err != 0) {
        perror("Error opening file");
        exit(-1);
    }

    logMutex = CreateMutex(NULL, FALSE, NULL);
    if (logMutex == NULL) {
        // Print the error to stderr
        fprintf(stderr, "Error: Unable to create mutex.\n");
        fclose(logFile);
        exit(-1);
    }
}

void write_log_format(const char* format, ...) {
    char buffer[4096]; // or whatever size you deem appropriate
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    write_log(buffer);
}

void write_log_uint64(const char* message, uint64_t value) {
    char buffer[4096];
    snprintf(buffer, sizeof(buffer), "%s: %llu", message, value);
    write_log(buffer);
}

void write_log(const char* message) {
    WaitForSingleObject(logMutex, INFINITE);  // Wait for ownership of the mutex.
    if (fprintf(logFile, "%s\n", message) < 0) {
        // Print the error to stderr if there's an issue writing to the log
        fprintf(stderr, "Error: Unable to write to log file.\n");
    }
    fflush(logFile);
    ReleaseMutex(logMutex);  // Release ownership of the mutex.
}

void close_logger() {
    if (logFile) {
        fclose(logFile);
    }
    if (logMutex) {
        CloseHandle(logMutex);
    }
}
