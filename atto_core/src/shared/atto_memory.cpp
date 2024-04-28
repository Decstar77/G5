#include "atto_defines.h"
#include "atto_memory.h"
#include <cstring>

namespace atto {
    MemoryArena ArenaAllocate( u64 size ) {
        MemoryArena a = {};
        a.memorySize = size;
        a.memory = ( u8* )PlatformAllocateMemory( size );
        return a;
    }

    void ArenaFree( MemoryArena * arena ) {
        memset( arena->memory, 0 , arena->memorySize );
        arena->memoryCurrent = 0;
    }

    void *  ArenaPush( MemoryArena * arena, u64 bytes ) {
        AssertMsg( arena->memoryCurrent + bytes < arena->memorySize, "Permanent memory overflow" );
        if( arena->memoryCurrent + bytes < arena->memorySize ) {
            void * result = arena->memory + arena->memoryCurrent;
            arena->memoryCurrent += bytes;

            memset( result, 0, bytes );

            return result;
        }

        return nullptr;
    }

    void MemSet( void * dst, i32 val, u64 size ) {
        memset( dst, val, size );
    }

    void MemCpy( void * dst, const void * src, u64 size ) {
        memcpy( dst, src, size );
    }

    static MemoryArena perm = {};
    static MemoryArena tran = {};
    
    void GlobalArenasMake( u64 permSize, u64 tranSize ) {
        perm = ArenaAllocate( permSize );
        tran = ArenaAllocate( tranSize );
    }

    void * MemoryAllocatePermanent( u64 size ) {
        return ArenaPush( &perm, size );
    }

    void * MemoryAllocateTransient( u64 size ) {
        return ArenaPush( &tran, size );
    }
    
    void MemoryClearTransient() {
        return ArenaFree( &tran );
    }
}
