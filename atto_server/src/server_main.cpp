
#include "../../atto_core/src/shared/atto_network.h"

#include "atto_server_logger.h"

#define ENET_IMPLEMENTATION
#include "../../vendor/enet/include/enet.h"

#include <iostream>
#include <unordered_map>

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ) {
        
    }
}

using namespace atto;

static std::unordered_map<u64, ENetPeer *> peers;

static u64 peerIdCounter = 0;
static u64 sessionIdCounter = 0;

struct PeerData {
    u64 peerId;
    u64 sessionId;
};

struct Session {
    u64 peer1;
    u64 peer2;
};

static void StartGameForPeers( const Session & session ) {
    {
        NetworkMessage msg = {};
        msg.type = NetworkMessageType::GAME_START;
        NetworkMessagePush<i32>( msg, 1 );
        NetworkMessagePush<i32>( msg, 2 );
        ENetPacket * packet = enet_packet_create( &msg, NetworkMessageGetTotalSize( msg ), ENET_PACKET_FLAG_RELIABLE );
        enet_peer_send( peers[ session.peer1 ], 0, packet );
    }
    {
        NetworkMessage msg = {};
        msg.type = NetworkMessageType::GAME_START;
        NetworkMessagePush<i32>( msg, 2 );
        NetworkMessagePush<i32>( msg, 1 );
        ENetPacket * packet = enet_packet_create( &msg, NetworkMessageGetTotalSize( msg ), ENET_PACKET_FLAG_RELIABLE );
        enet_peer_send( peers[ session.peer2 ], 0, packet );
    }
}

static std::unordered_map<u64, Session>    sessions;

int main( int argc, char * argv[] ) {

    Logger logger;

    if( enet_initialize() != 0 ) {
        logger.Error( "An error occurred while initializing ENet.\n" );
        return 0;
    }

    ENetAddress address = {};
    address.host = ENET_HOST_ANY;
    address.port = 27164;

    ENetHost *  server = enet_host_create( &address, 32, 2, 0, 0 );
    if( server == nullptr ) {
        logger.Error( "Failed to create ENet server host." );
        return 0;
    }

    logger.Info( "Server started..." );

    // Get ip
    char hostname[ 1024 ];
    hostname[ 1023 ] = '\0';
    gethostname( hostname, 1023 );
    logger.Info( "Hostname: %s", hostname );

    ENetEvent event = {};
    while( enet_host_service( server, &event, 30 ) >= 0 ) {
        switch( event.type ) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                logger.Info( "A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port );

                PeerData * peerData = new PeerData();
                event.peer->data = peerData;

                peers[ peerIdCounter ] = event.peer;
                peerData->peerId = peerIdCounter;

                peerIdCounter++;

                if( peerIdCounter == 2 ) {
                    logger.Info( "Started game!!" );

                    Session session = {};
                    session.peer1 = 0;
                    session.peer2 = 1;

                    ( (PeerData *)( peers[ 0 ]->data ) )->sessionId = sessionIdCounter;
                    ( (PeerData *)( peers[ 1 ]->data ) )->sessionId = sessionIdCounter;

                    StartGameForPeers( session );

                    sessions[ sessionIdCounter ] = session;
                    sessionIdCounter++;
                }

            } break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                //logger.Info( "ENET_EVENT_TYPE_RECEIVE.\n", event.peer->address.host, event.peer->address.port );
                
                u64 sid =( (PeerData *)( event.peer->data ) )->sessionId;
                Session & session = sessions[ sid ];

                if( session.peer1 == ( (PeerData *)( event.peer->data ) )->peerId ) {
                    enet_peer_send( peers[ session.peer2 ], 0, event.packet );
                }
                else {
                    enet_peer_send( peers[ session.peer1 ], 0, event.packet );
                }

                
                //Session * session = ( (PeerData *)( event.peer->data ) )->session;
                //if( session != nullptr ) {
                //    session->Recieve( event.peer, event.packet );
                //}

                //enet_packet_destroy( event.packet );
            } break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                logger.Info( "Disconnected.\n" );
            } break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            {
                logger.Info( "Disconnected due to timeout.\n" );
                //event.peer->data = NULL;
            } break;
            case ENET_EVENT_TYPE_NONE:
            {
            } break;
        }
    }


    return 0;

    //atto::Server* s = new atto::Server(27164, 32);
    //s->Run();
    //return 0;
}
