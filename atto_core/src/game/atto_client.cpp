#include "atto_client.h"
#include "../shared/atto_logging.h"
#include "../shared/atto_jobs.h"

#if 1
#define ENET_IMPLEMENTATION
#include <enet.h>

namespace atto {
    static NetworkStatus            status = NetworkStatus::NOT_STARTED;
    static ENetPeer *               peer = nullptr;
    static ENetHost *               client = nullptr;

    NetworkStatus NetworkCreateAndConnectNode() {
        if ( enet_initialize() != 0 ) {
            ATTOFATAL("An error occurred while initializing ENet" );
        }

        client = enet_host_create( NULL, 1, 2, 0, 0 );
        if ( client != nullptr ) {
            ENetAddress address = {};
            //if ( theGameSettings.serverIp.GetLength() != 0 ) {
            //    enet_address_set_host( &address, theGameSettings.serverIp.GetCStr() );
            //    //enet_address_set_host( &address, "77.37.120.37" );
            //} else {
            //    enet_address_set_host( &address, "127.0.0.1" );
            //}

            enet_address_set_host( &address, "127.0.0.1" );
            address.port = 27164;
        
            peer = enet_host_connect( client, &address, 2, 0 );
            if ( peer != nullptr ) {
                ENetEvent event = {};
                if ( enet_host_service( client, &event, 5000 ) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) {
                    status = NetworkStatus::RUNNING_AS_CLIENT;
                }
                else {
                    enet_peer_reset( peer );
                    enet_host_destroy( client );
                }
            }
            else {
                enet_host_destroy( client );
            }
        }
        else {
        }

        return status;
    }

    NetworkStatus NetworkUpdate() { return status; }
    void NetworkStop() {}
    void NetworkStartHost() {}
    void NetworkStartClient( const char * ip ) {}
    NetworkStatus NetworkGetStatus() { return status; }
    const char * NetworkGetStatusText() { return ""; }
    const char * NetworkGetIp() { return ""; }
    
    u32 NetworkGetPing() {
        if( status == NetworkStatus::RUNNING_AS_CLIENT ) {
            return peer->roundTripTime;
        }
        return 0;
    }

    void NetworkSend( const NetworkMessage & msg ) {
        i32 len = NetworkMessageGetTotalSize( msg );
        i32 flags = msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE;
        ENetPacket * packet = enet_packet_create( &msg, sizeof( NetworkMessage ), flags );
        enet_peer_send( peer, 0, packet );
    }

    bool NetworkRecieve( NetworkMessage & msg ) {
        if ( status != NetworkStatus::RUNNING_AS_CLIENT ) {
            return false;
        }

        ENetEvent event = {};
        i32 res = enet_host_service( client, &event, 0 );
        
        if ( res > 0 ) {
            if ( event.type == ENET_EVENT_TYPE_NONE ) {
        
            }
            else if ( event.type == ENET_EVENT_TYPE_CONNECT ) {
            }
            else if ( event.type == ENET_EVENT_TYPE_DISCONNECT ) {
                enet_peer_reset( peer );
                enet_host_destroy( client );
                status = NetworkStatus::NOT_STARTED;
            }
            else if ( event.type == ENET_EVENT_TYPE_RECEIVE ) {
                memcpy( &msg, ( NetworkMessage * )event.packet->data, sizeof( NetworkMessage ) );
                enet_packet_destroy( event.packet );
                return true;
            }
            else {
        
            }
        }
        else if ( res < 0 ) {
        }

        return false;
    }

}

#else

#include <ZeroTierSockets.h>

namespace atto {
    static NetworkStatus status = {};
    static u64  netId = 0x856127940c18578f;
    static u16  hostRemotePort = 0;
    static char hostRemoteAddr[ZTS_INET6_ADDRSTRLEN] = {};
    static int  hostLen = ZTS_INET6_ADDRSTRLEN;
    static int  fd = 0;
    static bool setAddr = false;
    static int  localAddrFamily = 0;
    char        localIpStr[ ZTS_IP_MAX_STR_LEN ] = {};

    class WaitForHostConnection : public enki::TaskSet {
    public:
        void ExecuteRange(  enki::TaskSetPartition range_, uint32_t threadnum_ ) override {
            fd = zts_tcp_server( "0.0.0.0", 27164, hostRemoteAddr, hostLen, &hostRemotePort );
            if ( fd < 0 ) {
                ATTOFATAL( "Network :: Could not create tpc server" );
            }

            ATTOTRACE( "Network :: Host started " );
        }
    };

    static WaitForHostConnection * waitForHostConnectionJob = nullptr;

    NetworkStatus NetworkCreateAndConnectNode() {
        //zts_init_from_storage("res/game/zt_ids/);
        zts_node_start();
        status = NetworkStatus::STARTING_NODE;
        return status;
    }

    NetworkStatus NetworkUpdate() {
        if ( status == NetworkStatus::STARTING_NODE ) {
            if ( zts_node_is_online() ) {
                status = NetworkStatus::ATTEMPTING_JOIN;
                //ATTOTRACE( "Network :: Node Id = %llx", (long long int)zts_node_get_id() );
                zts_net_join( netId );
            } else {
                //ATTOTRACE( "Network :: Attempting to create node" );
            }
        }

        if ( status == NetworkStatus::ATTEMPTING_JOIN ) {
            if ( zts_net_transport_is_ready( netId ) ) {
                status = NetworkStatus::WAITING_FOR_ADDRESS;
                //ATTOTRACE( "Network :: Transport ready ");
            } else {
                //ATTOTRACE( "Network :: Attempting to join network" );
            }
        }

        if ( status == NetworkStatus::LISTENING_FOR_CONNECTION ) {
            if ( waitForHostConnectionJob != nullptr ) {
                bool isComplete = waitForHostConnectionJob->GetIsComplete();
                if ( isComplete == true ) {
                    status = NetworkStatus::RUNNING_AS_HOST;
                }
            }
        }

        return status;
    }

    void NetworkStartHost() {
        if ( setAddr == false ) {
            setAddr = true;
            localAddrFamily = zts_util_get_ip_family( "0.0.0.0" );
        }
        
        if ( status == NetworkStatus::WAITING_FOR_ADDRESS ) {
            if ( zts_addr_is_assigned( netId, localAddrFamily ) ) {
                status = NetworkStatus::LISTENING_FOR_CONNECTION;
                zts_addr_get_str( netId, ZTS_AF_INET, localIpStr, ZTS_IP_MAX_STR_LEN);
                //ATTOTRACE( "Network :: Virtual IP %s", ipstr );
            }
            else {
                //ATTOTRACE( "Network :: Waiting for address" );
                return;
            }
        }

        if ( waitForHostConnectionJob == nullptr ) {
            waitForHostConnectionJob = new WaitForHostConnection();
            JobAdd( waitForHostConnectionJob );
        }
    }

    void NetworkStartClient( const char * ip ) {
        printf("Attempting to connect...\n");
        fd = zts_tcp_client( ip, 27164 );
        if ( fd < 0 ) {
            ATTOFATAL( "Network :: Could not create tpc server" );
        }

        status = NetworkStatus::RUNNING_AS_CLIENT;
    }
 
    void NetworkStop() {
        if ( status != NetworkStatus::NOT_STARTED ) {
            if ( status == NetworkStatus::LISTENING_FOR_CONNECTION ) {
                zts_close( fd );
            }

            zts_node_stop();
        }
    }

    const char * NetworkGetStatusText() {
        switch( status ) {
            case NetworkStatus::NOT_STARTED:
                return "Not Started";
            case NetworkStatus::STARTING_NODE:
                return "Starting Node";
            case NetworkStatus::ATTEMPTING_JOIN:
                return "Attempting Join";
            case NetworkStatus::WAITING_FOR_ADDRESS:
                return "Waiting for Address";
            case NetworkStatus::LISTENING_FOR_CONNECTION:
                return "Listening for connection";
            case NetworkStatus::RUNNING_AS_HOST:
                return "Running as Host";
            case NetworkStatus::RUNNING_AS_CLIENT:
                return "Running as Client";
            default:
                return "Unknown";
        }
    }

    NetworkStatus NetworkGetStatus() {
        return status;
    }
    
    void NetworkSend( const NetworkMessage & msg ) {
    }

    bool NetworkRecieve( NetworkMessage & msg ) {
        return false;
    }

    const char * NetworkGetIp() {
        return localIpStr;
    }
}

#endif