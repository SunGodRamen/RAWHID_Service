#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define MAX_LOG_SIZE 512

// Global log file pointer and mutex for thread safety.
extern FILE* logFile;
extern HANDLE logMutex;

void init_logger(char* filePath);
void write_log_format(const char* format, ...);
void write_log_uint64(const char* message, uint64_t value);
void write_log(const char* message);
void close_logger();

#endif // LOGGER_H
