#include "atto_server_logger.h"

#define ENET_IMPLEMENTATION
#include <enet.h>

#include "atto_server.h"

namespace atto {
    Server::Server(u16 port, u32 maxClients) :
        port(port),
        maxClients(maxClients)
    {
    }

    void Server::Run() {
        Logger logger = {};

        if (enet_initialize() != 0) {
            logger.Error("An error occurred while initializing ENet.\n");
        }

        ENetAddress address = {};
        address.host = ENET_HOST_ANY;
        address.port = port;

        server = enet_host_create(&address, maxClients, 2, 0, 0);
        if (server == nullptr) {
            logger.Error("Failed to create ENet server host.");
            return;
        }

        logger.Info("Server started...");

        ENetEvent event = {};
        while (enet_host_service(server, &event, 500) >= 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    logger.Info("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
                    /* Store any relevant client information here. */
                    event.peer->data = "Client information";
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    logger.Debug("A packet of length %lu containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        event.peer->data,
                        event.channelID);
                    
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    logger.Info("%s disconnected.\n", event.peer->data);
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    logger.Info("%s disconnected due to timeout.\n", event.peer->data);
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }

        enet_host_destroy(server);
        enet_deinitialize();
    }

}