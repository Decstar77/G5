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

    inline i32 GetTotalNetworkMessageSize( const NetworkMessage &msg ) {
        i32 of = offsetof( NetworkMessage, data );
        int size = of + msg.dataLen;
        return size;
    }
}

