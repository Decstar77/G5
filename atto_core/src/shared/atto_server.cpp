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
                case ENET_EVENT_TYPE_CONNECT: {
                    logger.Info("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);

                    PeerData* peerData = new PeerData();
                    peerData->serverIndex = peers.GetCount();
                    event.peer->data = peerData;
                    peers.Add(event.peer);

                    if (peers.GetCount() == 2) {
                        logger.Info("Started game!!");
                        Session *session = sessions.Add(Session(peers[0], peers[1], &logger));
                        ((PeerData*)peers[0]->data)->session = session;
                        ((PeerData*)peers[1]->data)->session = session;

                        session->StartGame();
                    }

                } break;
                case ENET_EVENT_TYPE_RECEIVE: {
                    logger.Debug("A packet of length %lu containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        event.peer->data,
                        event.channelID);

                     Session *session = ((PeerData*)(event.peer->data))->session;
                     if (session != nullptr) {
                         session->Recieve(event.peer, event.packet);
                     }

                    enet_packet_destroy(event.packet);
                } break;
                case ENET_EVENT_TYPE_DISCONNECT: {
                    logger.Info("%s disconnected.\n", event.peer->data);
                    event.peer->data = NULL;
                } break;
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    logger.Info("%s disconnected due to timeout.\n", event.peer->data);
                    event.peer->data = NULL;
                } break;
                case ENET_EVENT_TYPE_NONE: {
                } break;
            }
        }

        enet_host_destroy(server);
        enet_deinitialize();
    }

    Session::Session() {

    }

    Session::Session(ENetPeer* player1, ENetPeer* player2, Logger *logger) : logger(logger) {
        peers.Add(player1);
        peers.Add(player2);

        ((PeerData*)(player1->data))->sessionIndex = 0;
        ((PeerData*)(player2->data))->sessionIndex = 1;
    }

    void Session::StartGame() {
        
        const i32 peerCount = peers.GetCount();
        for (i32 peerIndex = 0; peerIndex < peerCount; peerIndex++) {
            ENetPeer* peer = peers[peerIndex];

            NetworkMessage msg = {};
            msg.type = NetworkMessageType::GAME_START;
            msg.playerNumber = peerIndex + 1;

            ENetPacket* actionPacket = enet_packet_create(
                &msg,
                sizeof(NetworkMessage),
                msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(peers[peerIndex], 0, actionPacket);
        }
    }

    void Session::Recieve(ENetPeer* sender, ENetPacket *packet) {
        i32 senderIndex = ((PeerData*)(sender->data))->sessionIndex;
        NetworkMessage msg = *(NetworkMessage*)packet->data;
        const i32 peerCount = peers.GetCount();
        for (i32 peerIndex = 0; peerIndex < peerCount; peerIndex++) {
            if (senderIndex == peerIndex) {
                continue;
            }

            ENetPacket* actionPacket = enet_packet_create(
                &msg,
                sizeof(NetworkMessage),
                msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(peers[peerIndex], 0, actionPacket);
        }
    }

    void Session::SendToPeers(NetworkMessage msg) {
        const i32 peerCount = peers.GetCount();
        for (i32 peerIndex = 0; peerIndex < peerCount; peerIndex++) {
            ENetPeer* peer = peers[peerIndex];
            ENetPacket* actionPacket = enet_packet_create(
                &msg,
                sizeof(NetworkMessage),
                msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE
            );
            
            enet_peer_send(peers[peerIndex], 0, actionPacket);
        }
    }

}