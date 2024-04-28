#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {
    enum class LogLevel {
        FATAL = 0,
        ERR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5,
        COUNT = 6
    };

    struct LoggingState {
        static constexpr u32            LOG_BUFFER_SIZE = 1024;
        char                            logBuffer[LOG_BUFFER_SIZE] = {};
        char                            outputBuffer[LOG_BUFFER_SIZE] = {};
        FixedList<LargeString, 10000>   logs = {};
    };

    void LoggerLogOutput( LogLevel level, const char * message, ... );

    // Logs a fatal-level message.
    #define ATTOFATAL(message, ...) atto::LoggerLogOutput(atto::LogLevel::FATAL, message, ##__VA_ARGS__);

#ifndef ATTOERROR
// Logs an error-level message.
    #define ATTOERROR(message, ...) atto::LoggerLogOutput(atto::LogLevel::ERR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
    #define ATTOWARN(message, ...) atto::LoggerLogOutput(atto::LogLevel::WARN, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define ATTOWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
    #define ATTOINFO(message, ...) atto::LoggerLogOutput(atto::LogLevel::INFO, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define ATTOINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
    #define ATTODEBUG(message, ...) atto::LoggerLogOutput(atto::LogLevel::DEBUG, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define ATTODEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
    #define ATTOTRACE(message, ...) atto::LoggerLogOutput(atto::LogLevel::TRACE, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define ATTOTRACE(message, ...)
#endif

}
