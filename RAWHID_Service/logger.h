#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

// Global log file pointer and mutex for thread safety.
extern FILE* logFile;
extern HANDLE logMutex;

typedef enum {
    _DEBUG = 1,
    _INFO,
    _WARN,
    _ERROR
} LogLevel;

void init_logger(char* filePath);
void set_log_level(LogLevel level);
void write_log_format(LogLevel level, const char* format, ...);
void write_log_uint64_dec(LogLevel level, const char* message, uint64_t value);
void write_log_uint64_bin(LogLevel level, const char* message, uint64_t value);
void write_log_uint64_hex(LogLevel level, const char* message, uint64_t value);
void write_log(LogLevel level, const char* message);
void close_logger();

#endif // LOGGER_H
