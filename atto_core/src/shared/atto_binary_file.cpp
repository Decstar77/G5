#include "atto_binary_file.h"
#include "atto_core.h"

namespace atto {
    void BinaryBlob::Create( const byte * buffer, i32 size ) {
        this->buffer = new byte[ size ];
        this->totalSize = size;
        this->current = 0;
    }

    void BinaryBlob::Save( Core * core, const char * path ) {
        core->ResourceWriteEntireBinaryFile( path, buffer, totalSize );
    }

    void BinaryBlob::Write( const void * data, i32 size ) {
        AssertMsg( current + size < totalSize, " BinaryFile::Write :: Network stuffies to big" );
        memcpy( buffer + current, data, size );
        current += size;
    }

    void BinaryBlob::Read( void * dst, i32 size ) {
        AssertMsg( current + size < totalSize, "BinaryFile::Read :: Network stuffies to big" );
        memcpy( dst, buffer + current, size );
        current += size;
    }
}
