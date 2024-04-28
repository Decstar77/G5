#pragma once

namespace atto {
    template<typename T> class GrowableList; template<u64 SizeBytes> class FixedStringBase; typedef FixedStringBase<256> LargeString; // CPP...

    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line );
    void PlatformLogMessage( const char * message, u8 colour );
    void PlatformErrorBox( const char * msg );
    u64  PlatformGetFileLastWriteTime( const char * fileName );
    void * PlatformAllocateMemory( u64 size );
    void  PlatformFreeMemory( void * buf );
    f64   PlatformGetCurrentTime();
    void PlatformReadEntireTextFile( const char * path, char * data, i32 maxLen );
    void PlatformWriteEntireTextFile( const char * path, const char * data );
    void PlatformReadEntireBinaryFile( const char * path, char * data, i32 maxLen );
    void PlatformWriteEntireBinaryFile( const char * path, const char * data, i32 size );
    i64 PlatformGetFileSize( const char * path );
    void PlatformRendererCreateTexture( class TextureResource * textureResource );
    void PlatformRendererCreateAudio( class AudioResource * audioResource );
    GrowableList<LargeString> PlatformGetAllFilesInFolderAndSubFolders( const char * folderPath, const char * filter );
}

