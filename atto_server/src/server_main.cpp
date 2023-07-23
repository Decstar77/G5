
//#include "../../atto_core/src/shared/atto_server.h"
#define ENET_IMPLEMENTATION
#include <enet.h>

#include <iostream>

int main( int argc, char * argv[] ) {

    ENetPeer * peer = nullptr;
    ENetHost * client = nullptr;

    if( enet_initialize() != 0 ) {
        return 0;
    }

    client = enet_host_create( NULL, 1, 2, 0, 0 );
    if( client == nullptr ) {
        return 0;
    }

    ENetAddress address = {};
    enet_address_set_host( &address, "127.0.0.1" );
    address.port = 27164;

    peer = enet_host_connect( client, &address, 2, 0 );
    if( peer == nullptr ) {
        return 0;
    }

    ENetEvent event = {};
    bool con = false;
    if( enet_host_service( client, &event, 5000 ) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) {
        con = true;
    }

    if( con ) {
        std::cout << "We connected " << std::endl;
        while( enet_host_service( client, &event, 30 ) >= 0 ) {
            
        }
    }

    return 0;

    //atto::Server* s = new atto::Server(27164, 32);
    //s->Run();
    //return 0;
}
