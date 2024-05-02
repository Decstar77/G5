
#include "../../atto_core/src/shared/atto_network.h"

#define ENET_IMPLEMENTATION
#include "../../vendor/enet/include/enet.h"

#include <iostream>
#include <unordered_map>

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ) {
        
    }
    void MemCpy( void * dst, const void * src, u64 size ) {
        memcpy( dst, src, size );
    }
}

using namespace atto;

static std::unordered_map<u64, ENetPeer *> peers;

static u64 peerIdCounter = 2;
static u64 sessionIdCounter = 1;

struct PeerData {
    u64 peerId = 0;
    u64 sessionId = 0;
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
    if( enet_initialize() != 0 ) {
        return 0;
    }

    ENetAddress address = {};
    address.host = ENET_HOST_ANY;
    address.port = 27164;

    ENetHost *  server = enet_host_create( &address, 32, 2, 0, 0 );
    if( server == nullptr ) {
        return 0;
    }

    // Get ip
    char hostname[ 1024 ];
    hostname[ 1023 ] = '\0';
    gethostname( hostname, 1023 );

    std::cout << "Server running" << std::endl;

    ENetEvent event = {};
    while( enet_host_service( server, &event, 30 ) >= 0 ) {
        switch( event.type ) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "Client connected" << std::endl;

                PeerData * peerData = new PeerData();
                event.peer->data = peerData;

                peers[ peerIdCounter ] = event.peer;
                peerData->peerId = peerIdCounter;

                peerIdCounter++;

                if( peerIdCounter % 2 == 0 ) {
                    std::cout << "Game Started" << std::endl;

                    Session session = {};
                    session.peer1 = peerIdCounter - 2;
                    session.peer2 = peerIdCounter - 1;

                    ( (PeerData *)( peers[ peerIdCounter - 2 ]->data ) )->sessionId = sessionIdCounter;
                    ( (PeerData *)( peers[ peerIdCounter - 1 ]->data ) )->sessionId = sessionIdCounter;

                    StartGameForPeers( session );

                    sessions[ sessionIdCounter ] = session;
                    sessionIdCounter++;
                }

            } break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                u64 sid =( (PeerData *)( event.peer->data ) )->sessionId;
                Session & session = sessions[ sid ];
                if( session.peer1 == ( (PeerData *)( event.peer->data ) )->peerId ) {
                    enet_peer_send( peers[ session.peer2 ], 0, event.packet );
                }
                else {
                    enet_peer_send( peers[ session.peer1 ], 0, event.packet );
                }
                //enet_packet_destroy( event.packet );
            } break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
            } break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            {
            } break;
            case ENET_EVENT_TYPE_NONE:
            {
            } break;
        }
    }

    return 0;
}