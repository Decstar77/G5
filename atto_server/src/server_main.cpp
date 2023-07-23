
#include "../../atto_core/src/shared/atto_network.h"

#include "atto_server_logger.h"

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <iostream>

using namespace atto;

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

    ENetEvent event = {};
    while( enet_host_service( server, &event, 30 ) >= 0 ) {
        switch( event.type ) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                logger.Info( "A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port );

                /*PeerData * peerData = new PeerData();
                peerData->serverIndex = peers.GetCount();
                event.peer->data = peerData;
                peers.Add( event.peer );

                if( peers.GetCount() == 2 ) {
                    logger.Info( "Started game!!" );
                    Session * session = sessions.Add( Session( peers[ 0 ], peers[ 1 ], &logger ) );
                    ( (PeerData *)peers[ 0 ]->data )->session = session;
                    ( (PeerData *)peers[ 1 ]->data )->session = session;

                    session->StartGame();
                }*/

            } break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
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
