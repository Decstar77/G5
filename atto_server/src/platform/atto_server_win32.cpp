#if 0
#include "atto_server_win32.h"
#include "../../atto_core/src/shared/atto_defines.h"

#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <Windows.h>

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ) {
        char buffer[ 1024 ];
        sprintf_s( buffer, 1024, "Assertion failed: %s\nFile: %s\nFunction: %s\nLine: %d\n", msg, file, func, line );
        OutputDebugStringA( buffer );
        MessageBoxA( NULL, buffer, "Assertion failed", MB_ICONERROR );
        DebugBreak();
    }

    inline static std::string WcharToAnsi( WCHAR const * str, size_t len ) {
        int const size_needed = WideCharToMultiByte( CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL );
        std::string strTo( size_needed, 0 );
        WideCharToMultiByte( CP_UTF8, 0, str, (int)len, &strTo[ 0 ], size_needed, NULL, NULL );
        return strTo;
    }

    void PlatformLogMessage( const char * message, u8 colour ) {
        HANDLE console_handle = GetStdHandle( STD_OUTPUT_HANDLE );
        // @NOTE: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
        static u8 levels[ 6 ] = { 64, 4, 6, 2, 1, 8 };
        SetConsoleTextAttribute( console_handle, levels[ colour ] );
        OutputDebugStringA( message );
        u64 length = strlen( message );
        LPDWORD number_written = 0;
        WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ),
                       message,
                       (DWORD)length,
                       number_written,
                       0
        );
        SetConsoleTextAttribute( console_handle, 8 );
    }

    void PlatformErrorBox( const char * msg ) {
        MessageBeep( MB_ICONERROR );
        MessageBoxA( NULL, msg, "Catastrophic Error !!! (BOOOM) ", MB_ICONERROR );
    }

    void * PlatformAllocateMemory( u64 size ) {
        return malloc( size );
    }

    void PlatformFreeMemory( void * buf ) {
        free( buf );
    }

    u64  PlatformGetFileLastWriteTime( const char * fileName ) {
        return 0;
    }

    void PlatformReadEntireTextFile( const char * path, char * data, i32 maxLen ) {}
    void PlatformWriteEntireTextFile( const char * path, const char * data ) {}
    void PlatformReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) {}
    void PlatformWriteEntireBinaryFile( const char * path, const char * data, i32 size ) {}
    i64 PlatformGetFileSize( const char * path ) { return 0; }
    void PlatformRendererCreateTexture( class TextureResource * textureResource ) {}
    void PlatformRendererCreateAudio( class AudioResource * audioResource ) {}
}
#endif