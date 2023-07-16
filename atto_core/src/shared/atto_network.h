#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"

namespace atto {
    enum class NetworkMessageType {
        NONE,
        GAME_START,
        GAME_UPDATE,
    };

    struct NetworkMessage {
        NetworkMessageType type;
        bool isUDP;
        i32 playerNumber;
        glm::vec2 p;
    };

    struct SS {
        i64 tick;
        glm::vec2 playerPos;
        glm::vec2 playerVel;
        
        u16 gen;
        u16 idx;
        glm::vec2 pos;
    };
}

