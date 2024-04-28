#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {
    template<i32 _size_>
    class FixedBinaryBlob {
    public:
        inline void Read( void * dst, i32 size ) {
            Assert( current + size < _size_ );
            MemCpy( dst, buffer + current, size );
            current += size;
        }
        
        inline i32 Write( const void * data, i32 size ) {
            Assert( current + size < _size_ );
            MemCpy( buffer + current, data, size );
            current += size;
            return size;
        }
        
        template<typename _type_>
        inline void Read( _type_ * obj ) {
            Read( (void *)obj, (i32)sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Write( const _type_ * obj ) {
            COMPILE_ASSERT( std::is_pointer<_type_>::value == false, "Highly likely bug to use BinaryFile::Push on pointer type!" );
            return Write( (void *)obj, (i32)sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Write( const Span<_type_> * span ) {
            Write( &span->count );
            return Write( (const void *)span->data, span->count * sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Write( const Span<const _type_> * span ) {
            Write( &span->count );
            return Write( (const void *)span->data, span->count * sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Read( const Span<_type_> * span ) {
            AssertMsg( false, "NoImpl" );
            return 0;
        }

        template<typename _type_>
        inline i32 Read( const Span<const _type_> * span ) {
            AssertMsg( false, "NoImpl" );
            return 0;
        }
        
        template<typename _type_, i32 cap>
        inline i32 Write( const FixedList<_type_, cap> * list ) {
            Span<const _type_> span = list->GetConstSpan();
            return Write( &span );
        }

        template<typename _type_, i32 cap>
        inline void Read( FixedList<_type_, cap> * list ) {
            AssertMsg( false, "NoImpl" );
        }

        template<typename _type_>
        inline i32 Write( const GrowableList<_type_> * list ) {
            Span<const _type_> span = list->GetConstSpan();
            return Write( &span );
        }

        template<typename _type_>
        inline void Read( GrowableList<_type_> * list ) {
            AssertMsg( false, "NoImpl" );
        }

        inline void Reset() {
            current = 0;
        }

        inline i32 GetSize() {
            return current;
        }

        char * GetBuffer() {
            return buffer;
        }

    private:
        i32 current;
        char buffer[ _size_ ];
    };

    class BinaryBlob {
    public:
    };

    
}

