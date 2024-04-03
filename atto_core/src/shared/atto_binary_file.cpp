#include "atto_binary_file.h"
#include "atto_core.h"

namespace atto {

    BinaryBlob::BinaryBlob() {
        buffer.reserve( Kilobytes( 1 ) );
    }

    void BinaryBlob::Save( Core * core, const char * path ) {
        core->ResourceWriteEntireBinaryFile( path, (char*)buffer.data(), (i32)buffer.size() );
    }

    void BinaryBlob::Write( const void * data, size_t size ) {
        const byte* byteData = static_cast<const byte*>(data);
        buffer.insert( buffer.end(), byteData, byteData + size );
    }

    void BinaryBlob::Read( void * dst, size_t size ) {
        Assert( current + size < buffer.size() );
        byte * p = &buffer.at( current );
        memcpy( dst, p, size );
    }
}
