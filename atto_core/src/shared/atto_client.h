#pragma once

#include "atto_containers.h"

namespace atto
{
    struct NetworkMessage {
        bool isUDP;
    };

    void            CreateNetworkState();
    void            DestroyNetworkState();

    void            NetworkConnect();
    bool            NetworkIsConnected();
    void            NetworkDisconnect();
    
    SmallString     NetworkStatusText();

    void            NetworkSend(const NetworkMessage &msg);
    bool            NetworkRecieve(NetworkMessage &msg);
}
