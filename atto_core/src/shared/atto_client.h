#pragma once

#include "atto_containers.h"

#include "atto_network.h"

struct ENetHost;
struct ENetPeer;

namespace atto {
    enum PlayerConnectState {
        PLAYER_CONNECTION_STATE_CONNECTING = 0,
        PLAYER_CONNECTION_STATE_SYNCHRONIZING,
        PLAYER_CONNECTION_STATE_RUNNING,
        PLAYER_CONNECTION_STATE_DISCONNECTED,
        PLAYER_CONNECTION_STATE_DISCONNECTING,
    };

    inline SmallString PlayerConnectStateToString( PlayerConnectState state ) {
        switch( state ) {
            case PLAYER_CONNECTION_STATE_CONNECTING: return SmallString::FromLiteral( "Connecting" );
            case PLAYER_CONNECTION_STATE_SYNCHRONIZING: return SmallString::FromLiteral( "Synchronizing" );
            case PLAYER_CONNECTION_STATE_RUNNING: return SmallString::FromLiteral( "Running" );
            case PLAYER_CONNECTION_STATE_DISCONNECTED: return SmallString::FromLiteral( "Disconnected" );
            case PLAYER_CONNECTION_STATE_DISCONNECTING: return SmallString::FromLiteral( "Disconnecting" );
            default: return SmallString::FromLiteral( "Unknown" );
        }
    }

    struct  ClientState {
        ENetPeer *              peer = nullptr;
        ENetHost *              client = nullptr;
        bool                    isConnected = false;
        SmallString             statusText = {};
    };
}
