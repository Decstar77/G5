
#include "atto_server_session.h"

#define ENET_IMPLEMENTATION
#include "../../vendor/enet/include/enet.h"

#include <iostream>
#include <unordered_map>

#include <GLFW/glfw3.h>

namespace atto {
    f64 PlatformGetCurrentTime() {
        return glfwGetTime();
    }
}

using namespace atto;

static std::unordered_map<u64, ENetPeer *> peers;
static std::unordered_map<u64, Session>    sessions;

static u64 peerIdCounter = 2;
static u64 sessionIdCounter = 1;

struct PeerData {
    u64 peerId = 0;
    u64 sessionId = 0;
};

static void StartGameForPeers( Session & session ) {
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

    session.Initialize();
}

static void SendToAllClientsInSession( Session & session, NetworkMessage * msg ) {
    ENetPacket * packet1 = enet_packet_create( msg, NetworkMessageGetTotalSize( *msg ), ENET_PACKET_FLAG_RELIABLE );
    enet_peer_send( peers[ session.peer1 ], 0, packet1 );
    ENetPacket * packet2 = enet_packet_create( msg, NetworkMessageGetTotalSize( *msg ), ENET_PACKET_FLAG_RELIABLE );
    enet_peer_send( peers[ session.peer2 ], 0, packet2 );
}

int main( int argc, char * argv[] ) {
    if( enet_initialize() != 0 ) {
        ATTOERROR( "An error occurred while initializing ENet.\n" );
        return 0;
    }

    if( glfwInit() == GLFW_FALSE ) {
        ATTOERROR( "An error occurred while initializing glfw.\n" );
        return 0;
    }

    ENetAddress address = {};
    address.host = ENET_HOST_ANY;
    address.port = 27164;

    ENetHost *  server = enet_host_create( &address, 32, 2, 0, 0 );
    if( server == nullptr ) {
        ATTOERROR( "Failed to create ENet server host." );
        return 0;
    }

    ATTOINFO( "Server started..." );

    // Get ip
    char hostname[ 1024 ];
    hostname[ 1023 ] = '\0';
    gethostname( hostname, 1023 );
    ATTOINFO( "Hostname: %s", hostname );
    
    f64 currentTime = glfwGetTime();
    f64 dt = 0;
    
    ENetEvent event = {};
    while( enet_host_service( server, &event, 10 ) >= 0 ) {
        f64 newTime = glfwGetTime();
        dt = newTime - currentTime;
        currentTime = newTime;

        // Loop through sessions
        for( auto & session : sessions ) {
            // Loop through entities
            session.second.Update( dt );
            NetworkMessage msg = {};
            msg.type = NetworkMessageType::ACTION_BUFFER;
            NetworkMessagePush<MapActionBuffer>( msg, session.second.outAction );
            SendToAllClientsInSession( session.second, &msg );
        }

        switch( event.type ) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                ATTOINFO( "A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port );

                PeerData * peerData = new PeerData();
                event.peer->data = peerData;

                peers[ peerIdCounter ] = event.peer;
                peerData->peerId = peerIdCounter;

                peerIdCounter++;

                ATTOINFO( "Peer counter %d", ( u32 )peerIdCounter );

                if( peerIdCounter % 2 == 0 ) {
                    ATTOINFO( "Started game!!" );

                    Session & session = sessions[ sessionIdCounter ];
                    session.peer1 = peerIdCounter - 2;
                    session.peer2 = peerIdCounter - 1;

                    ( (PeerData *)( peers[ peerIdCounter - 2 ]->data ) )->sessionId = sessionIdCounter;
                    ( (PeerData *)( peers[ peerIdCounter - 1 ]->data ) )->sessionId = sessionIdCounter;

                    StartGameForPeers( session );

                    sessionIdCounter++;
                }

            } break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                u64 sid =( (PeerData *)( event.peer->data ) )->sessionId;
                Session & session = sessions[ sid ];
                
                NetworkMessage * msg = (NetworkMessage *)event.packet->data;
                if( msg->type == NetworkMessageType::ACTION_BUFFER ) {
                    i32 offset = 0;
                    MapActionBuffer actionBuffer = NetworkMessagePop<MapActionBuffer>( *msg, offset );
                    session.inAction.Combine( actionBuffer );
                }

                //if( session.peer1 == ( (PeerData *)( event.peer->data ) )->peerId ) {
                //    enet_peer_send( peers[ session.peer2 ], 0, event.packet );
                //}
                //else {
                //    enet_peer_send( peers[ session.peer1 ], 0, event.packet );
                //}
                
                enet_packet_destroy( event.packet );
            } break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                ATTOINFO( "Disconnected.\n" );
            } break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            {
                ATTOINFO( "Disconnected due to timeout.\n" );
            } break;
            case ENET_EVENT_TYPE_NONE:
            {
            } break;
        }
    }

    return 0;
}
