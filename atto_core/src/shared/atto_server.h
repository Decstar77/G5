#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"

#include <enet.h>

namespace atto {
    
    class Server {
    public:
        Server(u16 port, u32 maxClients);
        ~Server();

        void Run();

    private:
        u16 port = 0;
        u32 maxClients = 0;

        ENetHost* server;
        //FixedList<ENetPeer, 2>      peers;
    };
    
}
