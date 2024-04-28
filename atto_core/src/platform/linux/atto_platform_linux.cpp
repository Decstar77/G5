
#if defined( __linux__ )

#include "../../shared/atto_defines.h"
#include <cstdlib>

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ){}
    void PlatformLogMessage( const char * message, u8 colour ) {}
    void PlatformErrorBox( const char * msg ) {}
    u64  PlatformGetFileLastWriteTime( const char * fileName ) { return 0; }
    void * PlatformAllocateMemory( u64 size ) {
        return malloc( size );
    }

    void PlatformFreeMemory( void * buf ) {
        free( buf );
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
