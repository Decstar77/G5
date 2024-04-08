#pragma once

#include "atto_containers.h"
#include "atto_containers_thread_safe.h"

#include "atto_network.h"

struct ENetHost;
struct ENetPeer;

namespace atto {
    struct  ClientState {
        ENetPeer *              peer = nullptr;
        ENetHost *              client = nullptr;
        bool                    isConnected = false;
        SmallString             statusText = {};
    };

}
