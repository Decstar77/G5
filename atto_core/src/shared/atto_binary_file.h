#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {
    class Core;

    template<i32 _size_>
    class FixedBinaryBlob {
    public:
        inline void Read( void * dst, i32 size ) {
            Assert( current + size < _size_ );
            memcpy( dst, buffer + current, size );
            current += size;
        }
        
        inline i32 Write( const void * data, i32 size ) {
            Assert( current + size < _size_ );
            memcpy( buffer + current, data, size );
            current += size;
            return size;
        }
        
        template<typename _type_>
        inline void Read( _type_ * obj ) {
            Read( (void *)obj, (i32)sizeof( _type_ ) );
        }

        template<typename _type_>
        inline i32 Write( const _type_ * obj ) {
            static_assert( std::is_pointer<_type_>::value == false, "Highly likely bug to use BinaryFile::Push on pointer type!" );
            return Write( (void *)obj, (i32)sizeof( _type_ ) );
        }

        template<typename _type_, i32 cap>
        inline i32 Write( const FixedList<_type_, cap> * list ) {
            const i32 count = list->GetCount();
            const i32 padding = 0; // @NOTE: Padding bytes to align the data
            Write( &count );
            Write( &padding );
            return Write( (const void *)list->GetData(), count * sizeof( _type_ ) );
        }

        template<typename _type_, i32 cap>
        inline void Read( FixedList<_type_, cap> * list ) {
            i32 count;
            Read( &count );
            i32 padding; // @NOTE: Padding bytes to align the data
            Read( &padding );
            list->Clear();

            // @SPEED:
            for( i32 i = 0; i < count; i++ ) {
                _type_ obj = {};
                Read( &obj );
                list->Add( obj );
            }
        }

        template<typename _type_>
        inline i32 Write( const GrowableList<_type_> * list ) {
            const i32 count = list->GetCount();
            const i32 capcity = list->GetCapcity();
            Write( &count );
            Write( &capcity );
            return Write( (const void *)list->GetData(), count * sizeof( _type_ ) );
        }

        template<typename _type_>
        inline void Read( GrowableList<_type_> * list ) {
            i32 count;
            Read( &count );
            i32 capcity; 
            Read( &capcity );
            list->Clear();

            // @SPEED:
            for( i32 i = 0; i < count; i++ ) {
                _type_ obj = {};
                Read( &obj );
                list->Add( obj );
            }
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
        BinaryBlob();
        inline void Reserve( size_t size ) { buffer.reserve( size ); }
        void Save( Core * core, const char * path );

        void Write( const void * data, size_t size );
        template<typename _type_>
        void Write( const _type_ * obj );

        inline void ReadPreare() { current = 0; }
        inline bool ReadComplete() { return current >= buffer.size(); }

        void Read( void * dst, size_t size );
        template<typename _type_>
        void Read( _type_ * obj );

    public:
        std::vector< byte > buffer;
        size_t              current = 0;
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

