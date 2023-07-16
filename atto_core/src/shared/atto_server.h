#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"

#include "atto_network.h"

#include <enet.h>

namespace atto {
    

    class Session;
    struct PeerData {
        i32 serverIndex;
        i32 sessionIndex;
        Session* session;
    };

    class Logger;
    class  Session {
    public:
        Session();
        Session(ENetPeer* player1, ENetPeer* player2, Logger* logger);

        void StartGame();
        void Recieve(ENetPeer* sender, ENetPacket* packet);
        void SendToPeers(NetworkMessage msg);

        FixedList<ENetPeer*, 2>      peers = {};
        Logger* logger = nullptr;
    };

    class Server {
    public:
        Server(u16 port, u32 maxClients);
        ~Server();

        void Run();

    private:
        u16 port = 0;
        u32 maxClients = 0;

        ENetHost* server = NULL;
        FixedList<ENetPeer*, 2>      peers = {};
        FixedList<Session, 64>       sessions = {};
    };
}
