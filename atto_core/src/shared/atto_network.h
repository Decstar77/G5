#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"

namespace atto {
    enum class NetworkMessageType {
        NONE,
        GAME_START,
        GGPO_MESSAGE,
    };

#define NETWORK_MESSAGE_MAX_BYTES 512
    struct NetworkMessage {
        NetworkMessageType type;
        bool isUDP;
        i32 dataLen;
        char data[ NETWORK_MESSAGE_MAX_BYTES ]; // Make sure this is last !!!
    };

    inline i32 NetworkMessageGetTotalSize( const NetworkMessage & msg ) {
        i32 of = offsetof( NetworkMessage, data );
        int size = of + msg.dataLen;
        return size;
    }

    inline void NetworkMessagePush( NetworkMessage & msg, void *data, i32 len ) {
        Assert( msg.dataLen + len < NETWORK_MESSAGE_MAX_BYTES, "Network stuffies to big" );
        memcpy( msg.data + msg.dataLen, data, len );
        msg.dataLen += len;
    }

    template<typename _type_>
    inline void NetworkMessagePush( NetworkMessage & msg, const _type_ & data ) {
        NetworkMessagePush( msg, (void *)&data, sizeof( _type_ ) );
    }
}

