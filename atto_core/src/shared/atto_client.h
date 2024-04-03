#pragma once

#include "atto_containers.h"
#include "atto_containers_thread_safe.h"

#include "atto_network.h"

#include <enet.h>

namespace atto
{
    class NetClient {
    public:
        NetClient(class Core* core);
        ~NetClient();

        void            Connect();
        bool            IsConnected();
        void            Disconnect();
        SmallString     StatusText();
        void            Send(const NetworkMessage& msg);
        bool            Recieve(NetworkMessage& msg);
        u32             GetPing();
        
    private:
        void            SetStatusStringInfo(const char* text);
        void            SetStatusStringErr(const char* text);
        void            NetworkLoop();
        
        std::atomic_bool     isRunning = false;
        std::atomic_bool     isConnected = false;
        std::atomic_bool     shouldConnect = false;
        std::atomic_bool     shouldDisconnect = false;

        std::atomic<SmallString> statusText = {};

        ENetAddress             address = {};
        ENetPeer*               peer = nullptr;
        ENetHost*               client = nullptr;
        Core* core = nullptr;

        FixedQueueThreadSafe<NetworkMessage, 1024> incommingMessages = {};
        FixedQueueThreadSafe<NetworkMessage, 1024> outgoingMessages = {};
    };
}
