#include "atto_core_windows.h"

#include <string>
#include <fstream>
#include <iostream>
#include <Windows.h>

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ) {
        char buffer[ 1024 ];
        sprintf_s( buffer, 1024, "Assertion failed: %s\nFile: %s\nFunction: %s\nLine: %d\n", msg, file, func, line );
        OutputDebugStringA( buffer );
        MessageBoxA( NULL, buffer, "Assertion failed", MB_ICONERROR );
        DebugBreak();
    }

    inline static std::string WcharToAnsi(WCHAR const* str, size_t len) {
        int const size_needed = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, str, (int)len, &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    void WindowsCore::OsLogMessage(const char* message, u8 colour) {
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        // @NOTE: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
        static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
        SetConsoleTextAttribute(console_handle, levels[colour]);
        OutputDebugStringA(message);
        u64 length = strlen(message);
        LPDWORD number_written = 0;
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE),
            message,
            (DWORD)length,
            number_written,
            0
        );
        SetConsoleTextAttribute(console_handle, 8);
    }

    void WindowsCore::OsErrorBox(const char* msg) {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(NULL, msg, "Catastrophic Error !!! (BOOOM) ", MB_ICONERROR);
    }

    static unsigned long long FileTimeToUInt64(const FILETIME& fileTime) {
        ULARGE_INTEGER dateTime;

        dateTime.LowPart = fileTime.dwLowDateTime;
        dateTime.HighPart = fileTime.dwHighDateTime;

        return dateTime.QuadPart;
    }

    u64 WindowsCore::OsGetFileLastWriteTime(const char* fileName) {
        FILETIME lastWriteTime = {};
        HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            LogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to open file %s. Error: %d", fileName, GetLastError());
        }
        else {
            if (GetFileTime(fileHandle, NULL, NULL, &lastWriteTime) == 0) {
                LogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to get file time, %s. Error: %d", fileName, GetLastError());
            }
            CloseHandle(fileHandle);
        }

        return FileTimeToUInt64(lastWriteTime);
    }


    void WindowsCore::WinBoyoWriteTextFile( const char * path, const char * text ) {
        std::ofstream file( path );
        if( file.is_open() ) {
            file << text;
            file.close();
        }
    }

    void WindowsCore::WinBoyoReadTextFile( const char * path, char * text, i32 maxLen ) {
        std::ifstream file( path );

        if( file.is_open() ) {
            file.seekg( 0, std::ios::end );
            std::streampos fileSize = file.tellg();
            file.seekg( 0, std::ios::beg );

            if( static_cast<i32>( fileSize ) > maxLen ) {
                std::cout << "DEBUG_ReadTextFile :: File size is too big. Truncating to " << maxLen << " bytes." << std::endl;
            }

            maxLen = Min( maxLen, static_cast<i32>( fileSize ) );

            file.read( text, maxLen );

            text[ maxLen ] = '\0';

            file.close();
        }
    }

    i64 WindowsCore::ResourceGetFileSize( const char * path ) {
        std::ifstream file( path );

        if( file.is_open() ) {
            file.seekg( 0, std::ios::end );
            std::streampos fileSize = file.tellg();
            file.close();
            return static_cast<i64>( fileSize );
        }

        return -1;
    }

    void WindowsCore::ResourceReadEntireFile( const char * path, char * data, i32 maxLen ) {
        WinBoyoReadTextFile( path, data, maxLen );
    }

    void WindowsCore::ResourceWriteEntireFile( const char * path, const char * data ) {
        WinBoyoWriteTextFile( path, data );
    }
}
