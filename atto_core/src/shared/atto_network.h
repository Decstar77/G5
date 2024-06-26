#pragma once

#include "atto_defines.h"

namespace atto {
    enum class NetworkMessageType {
        NONE,
        GAME_START,

        MAP_TURN,
        ECHO,
    };

#define NETWORK_MESSAGE_MAX_BYTES 512
    struct NetworkMessage {
        NetworkMessageType type;
        bool isUDP;
        i32 dataLen;
        char data[ NETWORK_MESSAGE_MAX_BYTES ]; // Make sure this is last !!!
    };

    inline i32 NetworkMessageGetTotalSize( const NetworkMessage & msg ) {
        i32 of = AttoOffsetOf( NetworkMessage, data );
        int size = of + msg.dataLen;
        return size;
    }

    inline void NetworkMessagePush( NetworkMessage & msg, void *data, i32 len ) {
        AssertMsg( msg.dataLen + len < NETWORK_MESSAGE_MAX_BYTES, "NetworkMessagePush :: Network stuffies to big" );
        MemCpy( msg.data + msg.dataLen, data, len );
        msg.dataLen += len;
    }

    template<typename _type_>
    inline void NetworkMessagePush( NetworkMessage & msg, const _type_ & data ) {
        static_assert( std::is_pointer_v<_type_> == false, "NetworkMessagePush :: Can't push pointers" );
        NetworkMessagePush( msg, (void *)&data, sizeof( _type_ ) );
    }

    template<typename _type_>
    inline _type_ NetworkMessagePop( NetworkMessage & msg, i32 & offset ) {
        AssertMsg( offset + sizeof( _type_ ) <= msg.dataLen, "NetworkMessagePop :: Network stuffies to big" );
        _type_ * data = (_type_ *)( msg.data + offset );
        offset += sizeof( _type_ );
        return *data;
    }

    inline void NetworkMessageReadTillEnd( NetworkMessage & msg, i32 & offset, void ** buffer, i32 & len ) {
        AssertMsg( offset < msg.dataLen, "NetworkMessageReadTillEnd :: Network stuffies to big" );
        *buffer = (void *)( msg.data + offset );
        len = msg.dataLen - offset;
        offset = msg.dataLen;
    }

}

