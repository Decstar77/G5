#define ENET_IMPLEMENTATION
#include <enet.h>

#include "atto_client.h"
#include "atto_core.h"

namespace atto {
    inline static void SetStatusStringInfo( ClientState & client, const char *text ) {
        client.statusText = SmallString::FromLiteral( text );
    }

    void Core::NetworkStart() {
        if ( enet_initialize() != 0 ) {
            LogOutput( LogLevel::FATAL, "An error occurred while initializing ENet" );
        }
    }

    void Core::NetworkConnect() {
        SetStatusStringInfo( client, "Attempting to connect." );
        client.client = enet_host_create( NULL, 1, 2, 0, 0 );
        if ( client.client != nullptr ) {
            ENetAddress address = {};
            if ( theGameSettings.serverIp.GetLength() != 0 ) {
                enet_address_set_host( &address, theGameSettings.serverIp.GetCStr() );
                //enet_address_set_host( &address, "77.37.120.37" );
            } else {
                enet_address_set_host( &address, "127.0.0.1" );
            }
            
            address.port = 27164;
        
            client.peer = enet_host_connect( client.client, &address, 2, 0 );
            if ( client.peer != nullptr ) {
                ENetEvent event = {};
                if ( enet_host_service( client.client, &event, 5000 ) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) {
                    SetStatusStringInfo( client,  "Connection succeeded." );
                    client.isConnected = true;
                }
                else {
                    SetStatusStringInfo( client, "Not connection to the server could be made" );
                    enet_peer_reset( client.peer );
                    enet_host_destroy( client.client );
                }
            }
            else {
                SetStatusStringInfo( client, "Failed to create enet peer!" );
                enet_host_destroy( client.client );
            }
        }
        else {
            SetStatusStringInfo( client, "Failed to create enet host!" );
        }
    }

    void Core::NetworkDisconnect() {
        SetStatusStringInfo( client, "Attempting to disconnect." );
        enet_peer_disconnect( client.peer, 0 );
        bool disconnected = false;

        ENetEvent event = {};
        while ( enet_host_service( client.client, &event, 3000 ) > 0 ) {
            switch ( event.type ) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy( event.packet );
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    SetStatusStringInfo( client, "Disconnection succeeded." );
                    disconnected = true;
                    break;
            }
        }
        
        // Drop connection, since disconnection didn't succeed
        if ( disconnected == false ) {
            enet_peer_reset( client.peer );
        }

        enet_host_destroy( client.client );
        client.isConnected = false;
    }

    void Core::NetworkSend( const NetworkMessage & msg ) {
        i32 len = NetworkMessageGetTotalSize( msg );
        i32 flags = msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE;
        ENetPacket * packet = enet_packet_create( &msg, sizeof( NetworkMessage ), flags );
        enet_peer_send( client.peer, 0, packet );
    }

    bool Core::NetworkRecieve( NetworkMessage & msg ) {
        if ( client.isConnected == false ) {
            return false;
        }

        ENetEvent event = {};
        i32 res = enet_host_service( client.client, &event, 0 );
        
        if ( res > 0 ) {
            if ( event.type == ENET_EVENT_TYPE_NONE ) {
        
            }
            else if ( event.type == ENET_EVENT_TYPE_CONNECT ) {
            }
            else if ( event.type == ENET_EVENT_TYPE_DISCONNECT ) {
                SetStatusStringInfo( client, "Connection to server has been lost" );
                enet_peer_reset( client.peer );
                enet_host_destroy( client.client );
                client.isConnected = false;
            }
            else if ( event.type == ENET_EVENT_TYPE_RECEIVE ) {
                //SetStatusStringInfo("Received packet");
                memcpy( &msg, ( NetworkMessage * )event.packet->data, sizeof( NetworkMessage ) );
                enet_packet_destroy( event.packet );
                return true;
            }
            else {
        
            }
        }
        else if ( res < 0 ) {
            SetStatusStringInfo( client, "Encountered error while polling the network" );
            //enet_peer_reset(peer);
            //enet_host_destroy(client);
        }

        return false;
    }

    bool Core::NetworkIsConnected() {
        return client.isConnected;
    }

    u32 Core::NetworkGetPing() {
        if( client.isConnected == true ) {
            return client.peer->roundTripTime;
        }
        return 0;
    }

    SmallString Core::NetworkGetStatusText() {
        return client.statusText;
    }

}

