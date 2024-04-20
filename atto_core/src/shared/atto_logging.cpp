#include "atto_logging.h"

#include "atto_core.h"

#include <iostream>
#include <string>
#include <stdarg.h> 

namespace atto {
    static LoggingState logger = {};

    static void StringFormatV(char* dest, size_t size, const char* format, va_list va_listp) {
        vsnprintf(dest, size, format, va_listp);
    }

    static void StringFormat(char* dest, size_t size, const char* format, ...) {
        va_list arg_ptr;
        va_start(arg_ptr, format);
        StringFormatV(dest, size, format, arg_ptr);
        va_end(arg_ptr);
    }

    void LoggerLogOutput( LogLevel level, const char * message, ... ) {
        const char* levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };
        const char* header = levelStrings[( u32 )level];

        memset( logger.logBuffer, 0, sizeof( logger.logBuffer ) );
        memset( logger.outputBuffer, 0, sizeof( logger.outputBuffer ) );

        va_list arg_ptr;
        va_start( arg_ptr, message );
        StringFormatV( logger.logBuffer, sizeof( logger.logBuffer ), message, arg_ptr );
        va_end( arg_ptr );

        StringFormat( logger.outputBuffer, sizeof( logger.outputBuffer ), "%s%s\n", header, logger.logBuffer );

        if (strlen( logger.outputBuffer ) < LargeString::CAPCITY ) {
            if (logger.logs.GetCount() == logger.logs.GetCapcity() ) {
                logger.logs.Clear();
            }
            logger.logs.Add( LargeString::FromLiteral( logger.outputBuffer ) );
        }

        PlatformLogMessage( logger.outputBuffer, ( u8 )level );

        if (level == LogLevel::FATAL ) {
            PlatformErrorBox( message );
        }
    }

    void Core::LogOutput( LogLevel level, const char* message, ... ) {
        const char* levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };
        const char* header = levelStrings[( u32 )level];

        memset( logger.logBuffer, 0, sizeof( logger.logBuffer ) );
        memset( logger.outputBuffer, 0, sizeof( logger.outputBuffer ) );

        va_list arg_ptr;
        va_start( arg_ptr, message );
        StringFormatV( logger.logBuffer, sizeof( logger.logBuffer ), message, arg_ptr );
        va_end( arg_ptr );

        StringFormat( logger.outputBuffer, sizeof( logger.outputBuffer ), "%s%s\n", header, logger.logBuffer );

        if (strlen( logger.outputBuffer ) < LargeString::CAPCITY ) {
            if (logger.logs.GetCount() == logger.logs.GetCapcity() ) {
                logger.logs.Clear();
            }
            logger.logs.Add( LargeString::FromLiteral( logger.outputBuffer ) );
        }

        PlatformLogMessage( logger.outputBuffer, ( u8 )level );

        if (level == LogLevel::FATAL ) {
            PlatformErrorBox( message );
        }
    }
}

