#include "atto_win32_core.h"

#include "native_file_dialog/nfd.h"

#include <backward/backward.hpp>

#include <string>
#include <fstream>
#include <sstream>
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

    void PlatformLogMessage(const char* message, u8 colour) {
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

    void PlatformErrorBox(const char* msg) {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(NULL, msg, "Catastrophic Error !!! (BOOOM) ", MB_ICONERROR);
    }

    static unsigned long long FileTimeToUInt64(const FILETIME& fileTime) {
        ULARGE_INTEGER dateTime;

        dateTime.LowPart = fileTime.dwLowDateTime;
        dateTime.HighPart = fileTime.dwHighDateTime;

        return dateTime.QuadPart;
    }

    u64 PlatformGetFileLastWriteTime( const char* fileName ) {
        FILETIME lastWriteTime = {};
        HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            LoggerLogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to open file %s. Error: %d", fileName, GetLastError());
        }
        else {
            if (GetFileTime(fileHandle, NULL, NULL, &lastWriteTime) == 0) {
                LoggerLogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to get file time, %s. Error: %d", fileName, GetLastError());
            }
            CloseHandle(fileHandle);
        }

        return FileTimeToUInt64(lastWriteTime);
    }

    // Function to write the call stack to a file
    //static void WriteMiniDump( EXCEPTION_POINTERS * pExceptionPointers ) {
    //    HANDLE hFile = CreateFile( L"crash_report.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    //    if( hFile != INVALID_HANDLE_VALUE ) {
    //        // Write the exception information
    //        EXCEPTION_RECORD * pExceptionRecord = pExceptionPointers->ExceptionRecord;
    //        CONTEXT * pContextRecord = pExceptionPointers->ContextRecord;

    //        std::wofstream fileStream( L"crash_report.txt" ); // Use wofstream constructor directly
    //        fileStream << L"Exception Code: 0x" << std::hex << pExceptionRecord->ExceptionCode << std::endl;

    //        // Write the call stack
    //        STACKFRAME64 stackFrame = {};
    //        CONTEXT context = *pContextRecord;
    //        HANDLE process = GetCurrentProcess();
    //        HANDLE thread = GetCurrentThread();
    //        DWORD machineType = IMAGE_FILE_MACHINE_AMD64; // Assuming 64-bit

    //        stackFrame.AddrPC.Mode = AddrModeFlat;
    //        stackFrame.AddrStack.Mode = AddrModeFlat;
    //        stackFrame.AddrFrame.Mode = AddrModeFlat;

    //        while( StackWalk64( machineType, process, thread, &stackFrame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL ) ) {
    //            DWORD64 address = stackFrame.AddrPC.Offset;
    //            if( address == 0 ) {
    //                break;
    //            }
    //            fileStream << L"0x" << std::hex << address << std::endl;
    //        }

    //        // Close file
    //        CloseHandle( hFile );
    //    }
    //}

    //// Function to create a minidump file
    //bool CreateMiniDump( EXCEPTION_POINTERS * pExceptionPointers, const std::wstring & dumpFileName ) {
    //    HANDLE hFile = CreateFileW( dumpFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    //    if( hFile == INVALID_HANDLE_VALUE ) {
    //        std::wcerr << L"Failed to create minidump file: " << dumpFileName << L". Error code: " << GetLastError() << std::endl;
    //        return false;
    //    }

    //    MINIDUMP_EXCEPTION_INFORMATION exceptInfo;
    //    exceptInfo.ThreadId = GetCurrentThreadId();
    //    exceptInfo.ExceptionPointers = pExceptionPointers;
    //    exceptInfo.ClientPointers = FALSE;

    //    BOOL success = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exceptInfo, NULL, NULL );
    //    CloseHandle( hFile );

    //    if( !success ) {
    //        std::wcerr << L"Failed to write minidump file: " << dumpFileName << L". Error code: " << GetLastError() << std::endl;
    //        return false;
    //    }

    //    return true;
    //}

    //LONG WINAPI CustomExceptionHandler( EXCEPTION_POINTERS * pExceptionPointers ) {
    //    std::wstring dumpFileName = L"crash_dump.dmp";
    //    if( CreateMiniDump( pExceptionPointers, dumpFileName ) ) {
    //        std::wcout << L"Minidump created: " << dumpFileName << std::endl;
    //    }
    //    else {
    //        std::wcerr << L"Failed to create minidump." << std::endl;
    //    }

    //    // Let the system handle the exception
    //    return EXCEPTION_CONTINUE_SEARCH;
    //}

    static void onUnhandledException() {
        std::cerr << "Unhandled exception occurred! Generating crash dump..." << std::endl;

        backward::StackTrace st;
        st.load_here();

        backward::Printer p;
        p.print( st );
    }

    void WindowsCore::Win32SetupCrashReporting() {
        static backward::SignalHandling sh;
        
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

void WindowsCore::WinBoyoWriteBinaryFile( const char * path, const char * data, i64 size ) {
        std::ofstream file( path, std::ios::out | std::ios::binary );
        if( file.is_open() ) {
            file.write( data, size );
            file.close();
        }
    }

    void WindowsCore::WinBoyoReadBinaryFile( const char * path, char * data, i64 size ) {
        std::ifstream file( path, std::ios::in | std::ios::binary );
        if( file.is_open() ) {
            file.read( data, size );
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
        else {
        }

        return -1;
    }

    void WindowsCore::ResourceReadEntireTextFile( const char * path, char * data, i32 maxLen ) {
        WinBoyoReadTextFile( path, data, maxLen );
    }

    void WindowsCore::ResourceWriteEntireTextFile( const char * path, const char * data ) {
        WinBoyoWriteTextFile( path, data );
    }

    void WindowsCore::ResourceReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) {
        WinBoyoReadBinaryFile( path, reinterpret_cast<char *>( data ), maxLen );
    }

    void WindowsCore::ResourceWriteEntireBinaryFile( const char * path, const char * data, i32 size ) {
        WinBoyoWriteBinaryFile( path, reinterpret_cast<const char *>( data ), size );
    }

    bool WindowsCore::WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) {
        nfdchar_t * outPath = NULL;
        nfdresult_t result = NFD_OpenDialog( filter, basePath, &outPath );

        if( result == NFD_OKAY ) {
            res.Clear();
            res.Add( outPath );
            res.BackSlashesToSlashes();
            free( outPath );
            return true;
        }
        else {
            LogOutput( LogLevel::ERR, "Error: %s\n", NFD_GetError() );
        }

        return false;
    }

    bool WindowsCore::WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) {
        nfdchar_t * outPath = NULL;
        nfdresult_t result = NFD_PickFolder( basePath, &outPath );

        if( result == NFD_OKAY ) {
            res.Clear();
            res.Add( outPath );
            res.BackSlashesToSlashes();
            free( outPath );
            return true;
        }
        else {
            LogOutput( LogLevel::ERR, "Error: %s\n", NFD_GetError() );
        }

        return false;
    }

    
}
