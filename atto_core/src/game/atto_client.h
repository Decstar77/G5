#pragma once

#include "../shared/atto_containers.h"
#include "../shared/atto_network.h"

namespace atto {

    enum class NetworkStatus {
        NOT_STARTED = 0,
        STARTING_NODE,
        ATTEMPTING_JOIN,
        WAITING_FOR_ADDRESS,
        LISTENING_FOR_CONNECTION,
        RUNNING_AS_HOST,
        RUNNING_AS_CLIENT,
    };

    NetworkStatus                       NetworkCreateAndConnectNode();
    NetworkStatus                       NetworkUpdate();
    void                                NetworkStop();
    void                                NetworkStartHost();
    void                                NetworkStartClient( const char * ip );
    NetworkStatus                       NetworkGetStatus();
    const char *                        NetworkGetStatusText();
    void                                NetworkSend( const NetworkMessage & msg );
    bool                                NetworkRecieve( NetworkMessage & msg );
    const char *                        NetworkGetIp();
    u32                                 NetworkGetPing();
}
