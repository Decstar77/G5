#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {
    class Core;

    class BinaryBlob {
    public:
        void Create( const byte * buffer, i32 size );
        void Save( Core * core, const char * path );

        void Write( const void * data, i32 size );
        template<typename _type_>
        void Write( const _type_ * obj );

        void Read( void * dst, i32 size );
        template<typename _type_>
        void Read( _type_ * obj );

    public:
        byte * buffer = nullptr;
        i32 totalSize = 0;
        i32 current = 0;
    };

    template<typename _type_>
    void BinaryBlob::Write( const _type_ * obj ) {
        static_assert( std::is_pointer<_type_>::value == false, "Highly likely bug to use BinaryFile::Push on pointer type!" );
        Write( (void *)obj, (i32)sizeof( _type_ ) );
    }

    template<typename _type_>
    void BinaryBlob::Read( _type_ * obj ) {
        Read( (void *)obj, (i32)sizeof( _type_ ) );
    }
}

