#define ENET_IMPLEMENTATION
#include <enet.h>

#include "atto_client.h"
#include "atto_core.h"

namespace atto {
    void NetClient::SetStatusStringInfo( const char * text ) {
        core->LogOutput( LogLevel::INFO, text );
        statusText = SmallString::FromLiteral( text );
    }

    void NetClient::SetStatusStringErr( const char * text ) {
        core->LogOutput( LogLevel::ERR, text );
        statusText = SmallString::FromLiteral( text );
    }

    void NetClient::NetworkLoop() {
        core->LogOutput( LogLevel::INFO, "Network thread starting" );

        if( enet_initialize() != 0 ) {
            core->LogOutput( LogLevel::FATAL, "An error occurred while initializing ENet" );
            return;
        }

        isRunning = true;

        while( isRunning ) {
            if( !isConnected && shouldConnect ) {
                SetStatusStringInfo( "Attempting to connect." );
                client = enet_host_create( NULL, 1, 2, 0, 0 );
                if( client != nullptr ) {
                    enet_address_set_host( &address, "127.0.0.1" );
                    address.port = 27164;

                    peer = enet_host_connect( client, &address, 2, 0 );
                    if( peer != nullptr ) {
                        ENetEvent event = {};
                        if( enet_host_service( client, &event, 5000 ) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) {
                            SetStatusStringInfo( "Connection succeeded." );
                            isConnected = true;
                        }
                        else {
                            SetStatusStringInfo( "Not connection to the server could be made" );
                            enet_peer_reset( peer );
                            enet_host_destroy( client );
                        }
                    }
                    else {
                        SetStatusStringErr( "Failed to create enet peer!" );
                        enet_host_destroy( client );
                    }
                }
                else {
                    SetStatusStringErr( "Failed to create enet host!" );
                }

                shouldConnect = false;
            }

            while( isConnected ) {
                if( shouldDisconnect ) {
                    enet_peer_disconnect( peer, 0 );
                    bool disconnected = false;

                    ENetEvent event = {};
                    while( enet_host_service( client, &event, 3000 ) > 0 ) {
                        switch( event.type ) {
                            case ENET_EVENT_TYPE_RECEIVE:
                                enet_packet_destroy( event.packet );
                                break;
                            case ENET_EVENT_TYPE_DISCONNECT:
                                SetStatusStringInfo( "Disconnection succeeded." );
                                disconnected = true;
                                break;
                        }
                    }

                    // Drop connection, since disconnection didn't succeed
                    if( !disconnected ) {
                        enet_peer_reset( peer );
                    }

                    enet_host_destroy( client );

                    isConnected = false;
                    shouldDisconnect = false;

                    break;
                }

                ENetEvent event = {};
                i32 res = enet_host_service( client, &event, 50 );

                if( res > 0 ) {
                    if( event.type == ENET_EVENT_TYPE_NONE ) {

                    }
                    else if( event.type == ENET_EVENT_TYPE_CONNECT ) {
                    }
                    else if( event.type == ENET_EVENT_TYPE_DISCONNECT ) {
                        SetStatusStringInfo( "Connection to server has been lost" );
                        enet_peer_reset( peer );
                        enet_host_destroy( client );
                        isConnected = false;
                        break;
                    }
                    else if( event.type == ENET_EVENT_TYPE_RECEIVE ) {
                        //SetStatusStringInfo("Received packet");
                        NetworkMessage * msg = (NetworkMessage *)event.packet->data;
                        incommingMessages.Enqueue( *msg );

                        enet_packet_destroy( event.packet );
                    }
                    else {

                    }
                }
                else if( res < 0 ) {
                    SetStatusStringErr( "Encountered error while polling the network" );
                    //enet_peer_reset(peer);
                    //enet_host_destroy(client);
                }

                while( outgoingMessages.IsEmpty() == false ) {
                #if 0 
                    NetworkMessage msg = outgoingMessages->Dequeue();
                    NetworkMessageBuffer msgBuf = {};
                    CompileNetworkMessage( msg, msgBuf );
                    ATTOINFO( "Sending %d bytes", msgBuf.GetHeadLocation() );
                    ENetPacket * packet = enet_packet_create( msgBuf.GetData(), msgBuf.GetHeadLocation(), msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE );
                    enet_peer_send( peer, 0, packet );
                #else
                    NetworkMessage msg = outgoingMessages.Dequeue();
                    i32 len = NetworkMessageGetTotalSize( msg );
                    i32 flags = msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE;
                    ENetPacket * packet = enet_packet_create( &msg, len, flags );
                    enet_peer_send( peer, 0, packet );
                #endif

                }

                //ATTOINFO("Enet ping %d", peer->roundTripTime);
            }
        }

        enet_deinitialize();

        //ATTOINFO("Network thread finished");
    }

    NetClient::NetClient( class Core * inCore ) {
        core = inCore;
        std::thread( &NetClient::NetworkLoop, this ).detach();
    }

    NetClient::~NetClient() {
        Disconnect();
        isRunning = false;
    }

    void NetClient::Connect() {
        if( !isConnected ) {
            shouldConnect = true;
        }
    }

    bool NetClient::IsConnected() {
        return isConnected.load();
    }

    void NetClient::Disconnect() {
        if( isConnected ) {
            shouldDisconnect = true;
        }
    }

    SmallString NetClient::StatusText() {
        SmallString copy = statusText.load();
        return copy;
    }

    void NetClient::Send( const NetworkMessage & msg ) {
        if( isConnected ) {
            outgoingMessages.Enqueue( msg );
        }
    }

    bool NetClient::Recieve( NetworkMessage & msg ) {
        if( incommingMessages.IsEmpty() ) {
            return false;
        }

        msg = incommingMessages.Dequeue();

        return true;
    }

    u32 NetClient::GetPing() {
        if( isConnected ) {
            return peer->roundTripTime;
        }
        else {
            return 0;
        }
    }

}

