#pragma once
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <time.h>

namespace atto {

    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    class Logger {
    public:
        Logger() {
            std::time_t t = std::time(nullptr);
            std::tm now = {};
        //#if _WIN32
        //    localtime_s(&now, &t);
        //#else 
        //    localtime_r( &now, &t );
        //#endif

            std::stringstream filename;
            filename << "logs/" << std::put_time(&now, "%Y-%m-%d_%H-%M-%S") << ".txt";

            fileStream.open(filename.str(), std::ios::app);  // Open the log file in append mode
        }

        ~Logger() {
            fileStream.close();
        }

        inline void Debug(const char* format, ...) {
            va_list args;
            va_start(args, format);
            Log(LogLevel::DEBUG, format, args, std::cout);
            va_end(args);
        }

        inline void Info(const char* format, ...) {
            va_list args;
            va_start(args, format);
            Log(LogLevel::INFO, format, args, std::cout);
            va_end(args);
        }

        inline void Warning(const char* format, ...) {
            va_list args;
            va_start(args, format);
            Log(LogLevel::WARNING, format, args, std::cerr);
            va_end(args);
        }

        inline void Error(const char* format, ...) {
            va_list args;
            va_start(args, format);
            Log(LogLevel::ERROR, format, args, std::cerr);
            va_end(args);
        }

        inline void Fatal(const char* format, ...) {
            va_list args;
            va_start(args, format);
            Log(LogLevel::FATAL, format, args, std::cerr);
            va_end(args);
        }

        inline void Dump() {
            // Dump any buffered logs to the log file immediately
            fileStream << logBuffer.str();
            logBuffer.str("");  // Clear the log buffer
        }

    private:
        inline void Log(LogLevel level, const char* format, va_list args, std::ostream& stream) {
            std::time_t t = std::time(nullptr);
            std::tm now = { 0 };

        //#if _WIN32
        //    localtime_s( &now, &t );
        //#else 
        //    localtime_r( &now, &t );
        //#endif

            const char* levelString = GetLevelString(level);

            // Print the log level and date
            stream << "[" << std::put_time(&now, "%Y-%m-%d %H:%M:%S") << "] [" << levelString << "] ";

            // Format the message using variadic arguments
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);

            // Print the formatted message
            stream << GetLevelStringSpaces(level) << buffer << std::endl;

            // Append the log to the log buffer
            logBuffer << "[" << std::put_time(&now, "%Y-%m-%d %H:%M:%S") << "] [" << levelString << "] " << GetLevelStringSpaces(level) << buffer << std::endl;
        }

        inline const char* GetLevelString(LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARNING:
                    return "WARNING";
                case LogLevel::ERROR:
                    return "ERROR";
                case LogLevel::FATAL:
                    return "FATAL";
                default:
                    return "UNKNOWN";
            }
        }

        inline const char* GetLevelStringSpaces(LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG:
                    return "   ";
                case LogLevel::INFO:
                    return "   ";
                case LogLevel::WARNING:
                    return "";
                case LogLevel::ERROR:
                    return "  ";
                case LogLevel::FATAL:
                    return "  ";
                default:
                    return " ";
            }
        }

        std::ofstream fileStream;  // Log file stream
        std::stringstream logBuffer;  // Buffer to store logs until Dump() is called
    };
}