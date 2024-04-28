#pragma once

namespace atto {
    struct MemoryArena {
        u8 *    memory;
        u64     memorySize;
        u64     memoryCurrent;
    };

    MemoryArena                         ArenaAllocate( u64 size );
    void                                ArenaFree( MemoryArena * arena );
    void *                              ArenaPush( MemoryArena * arena, u64 size );

    // Handy dandy funcs
    void                                MemSet( void * dst, i32 val, u64 size );
    void                                MemCpy( void * dst, const void * src, u64 size );

    // Global areans
    void                                GlobalArenasMake( u64 permSize, u64 tranSize );
    void *                              MemoryAllocatePermanent( u64 size );
    void *                              MemoryAllocateTransient( u64 size );
    void                                MemoryClearTransient();

    template<typename _type_>
    inline _type_ * MemoryAllocatePermanent() {
        return (_type_ *)MemoryAllocatePermanent( sizeof( _type_ ) );
    }

    template<typename _type_>
    inline _type_ * MemoryAllocateTransient() {
        return (_type_ *)MemoryAllocateTransient( sizeof( _type_ ) );
    }
}
