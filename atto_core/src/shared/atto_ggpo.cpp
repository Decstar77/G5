#include "atto_ggpo.h"
#include "atto_core.h"
#include "atto_network.h"
#include "atto_client.h"

namespace atto {

    static Core * theCore = nullptr;
    static bool BeginGame( const char * game ) {
        return true;
    }

    static bool AdvanceFrame( int flags ) {
        i32 inputs[ MP_MAX_INPUTS ] = {};
        int dcFlags = 0;;

        ggpo_synchronize_input( theCore->GetMPState()->session, (void *)inputs, sizeof( i32 ) * MP_MAX_INPUTS, &dcFlags );

        theCore->GetSimLogic()->Advance( inputs[ 0 ], inputs[ 1 ], dcFlags );

        ggpo_advance_frame( theCore->GetMPState()->session );

        return true;
    }

    static bool LoadGameState( unsigned char * buffer, int len ) {
        theCore->GetSimLogic()->LoadState( buffer, len );
        return true;
    }

    static bool SaveGameState( unsigned char ** buffer, int * len, int * checksum, int frame ) {
        theCore->GetSimLogic()->SaveState( buffer, len, checksum, frame );
        return true;
    }

    static void FreeBuffer( void * buffer ) {
        theCore->GetSimLogic()->FreeState( buffer );
    }

    static bool OnEvent( GGPOEvent * info ) {
        switch( info->code ) {
            case GGPO_EVENTCODE_CONNECTED_TO_PEER:
            {
                theCore->GetMPState()->SetConnectionState( info->u.connected.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, 0, 0 );
            } break;
            case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
            {
                int p = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
                theCore->GetMPState()->SetConnectionState( info->u.synchronizing.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, p, 0 );
            }break;
            case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
            {
                theCore->LogOutput( LogLevel::INFO, "Synchronized with peer %d", info->u.synchronized.player );
                theCore->GetMPState()->SetConnectionState( info->u.synchronized.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, 100, 0 );
            } break;
            case GGPO_EVENTCODE_RUNNING:
            {
                theCore->LogOutput( LogLevel::INFO, "Running");
                theCore->GetMPState()->SetConnectionState( PLAYER_CONNECTION_STATE_RUNNING, 0, 0 );
            } break;
            case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
            {
                theCore->LogOutput( LogLevel::INFO, "Interupted" );
                int n = (int)theCore->GetLastTime();
                int d = info->u.connection_interrupted.disconnect_timeout;
                theCore->GetMPState()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_DISCONNECTING, n, d );
            } break;
            case GGPO_EVENTCODE_CONNECTION_RESUMED:
            {
                theCore->LogOutput( LogLevel::INFO, "Resumed" );
                theCore->GetMPState()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_RUNNING, 0, 0 );
            }
            break;
            case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
            {
                theCore->LogOutput( LogLevel::INFO, "DC from peer" );
                theCore->GetMPState()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_DISCONNECTED, 0, 0 );
            } break;
            case GGPO_EVENTCODE_TIMESYNC:
            {
                theCore->GetSimLogic()->SkipNextUpdates( info->u.timesync.frames_ahead );
            } break;
        }
        return true;
    }

    static bool LogGameState( char * filename, unsigned char * buffer, int len ) {
        theCore->GetSimLogic()->LogState( filename, buffer, len );
        return true;
    }

    static void NetworkSendTo( GGPOPeerHandle local, GGPOPeerHandle peer, void * buffer, int len ) {
        NetworkMessage msg = {};
        msg.isUDP = true;
        msg.type = NetworkMessageType::GGPO_MESSAGE;
        NetworkMessagePush( msg, local );
        NetworkMessagePush( msg, peer );
        NetworkMessagePush( msg, buffer, len );

        theCore->GetNetClient()->Send( msg );
    }

    static bool NetworkPoll( GPPONetworkCallbacks * msg ) {
        FixedQueue<NetworkMessage, 1024> & messages = theCore->GetGGPOMessages();

        while( messages.IsEmpty() == false ) {
            NetworkMessage m = messages.Dequeue();
            Assert( m.type == NetworkMessageType::GGPO_MESSAGE, "Msg is not for GGPO?" );

            i32 offset = 0;
            GGPOPeerHandle sender = NetworkMessagePop<GGPOPeerHandle>( m, offset );
            GGPOPeerHandle recipient = NetworkMessagePop<GGPOPeerHandle>( m, offset );

            i32 len = 0;
            void * buffer = nullptr;
            NetworkMessageReadTillEnd( m, offset, &buffer, len );

            msg->OnMsg( sender, buffer, len );
        }

        return true;
    }

    void Core::GGPOStartSession( i32 local, i32 peer ) {
        theCore = this;

        Assert( theCore != nullptr, "The core is null ?" );
        Assert( theCore->mpState.session == nullptr, "Session already started ? " );

        LogOutput( LogLevel::INFO, "Starting GGPO Session" );

        GGPOSessionCallbacks cb = { 0 };
        cb.begin_game = BeginGame;
        cb.advance_frame = AdvanceFrame;
        cb.load_game_state = LoadGameState;
        cb.save_game_state = SaveGameState;
        cb.free_buffer = FreeBuffer;
        cb.on_event = OnEvent;
        cb.log_game_state = LogGameState;
        cb.network_send_to = NetworkSendTo;
        cb.network_poll = NetworkPoll;

        i32 suc = ggpo_start_session( &mpState.session, &cb, "Big chongus", 2, sizeof( int ), local );

        Assert( suc == 0, "GGPO start session failed " );
        if( suc == 0 ) {
            LogOutput( LogLevel::INFO, "GGPO Session started " );
        }
        else {
            LogOutput( LogLevel::ERR, "GGPO Session could not be started" );
            mpState.session = nullptr;
            return;
        }

        ggpo_set_disconnect_timeout( mpState.session, 3000 );
        ggpo_set_disconnect_notify_start( mpState.session, 1000 );

        {
            GGPOPlayer localPlayer = {};
            localPlayer.size = sizeof( GGPOPlayer );
            localPlayer.type = GGPO_PLAYERTYPE_LOCAL;
            localPlayer.player_num = local;
            localPlayer.peer = local;

            GGPOPlayerHandle localHandle = {};
            suc = ggpo_add_player( mpState.session, &localPlayer, &localHandle );
            Assert( suc == 0, "Could not add local player" );

            theCore->mpState.local = {};
            theCore->mpState.local.playerHandle = localHandle;
            theCore->mpState.local.state = PLAYER_CONNECTION_STATE_CONNECTING;
            theCore->mpState.local.connectProgress = 0;
            theCore->mpState.local.disconnectStart = 0;
            theCore->mpState.local.disconnectTimeout = 0;

            ggpo_set_frame_delay( mpState.session, localHandle, MP_FRAME_DELAY );
        }

        {
            GGPOPlayer peerPlayer = {};
            peerPlayer.size = sizeof( GGPOPlayer );
            peerPlayer.type = GGPO_PLAYERTYPE_REMOTE;
            peerPlayer.player_num = peer;
            peerPlayer.peer = peer;

            GGPOPlayerHandle peerHandle = {};
            suc = ggpo_add_player( mpState.session, &peerPlayer, &peerHandle );
            Assert( suc == 0, "Could not add peer player" );

            theCore->mpState.peer = {};
            theCore->mpState.peer.connectProgress = 0;
            theCore->mpState.peer.playerHandle = peerHandle;
            theCore->mpState.peer.disconnectStart = 0;
            theCore->mpState.peer.disconnectTimeout = 0;
        }
    }

    void Core::GGPOPoll() {
        Assert( mpState.session != nullptr, "Session not started que ?" );
        ggpo_idle( mpState.session );
    }

    void MultiplayerState::SetConnectionState( PlayerConnectState state, i32 arg1, i32 arg2 ) {
        for( i32 i = 0; i < MP_PLAYER_COUNT; i++ ) {
            players[ i ].state = state;
        }
    }

    void MultiplayerState::SetConnectionState( int playerNumber, PlayerConnectState state, i32 arg1, i32 arg2 ) {
        for( i32 i = 0; i < MP_PLAYER_COUNT; i++ ) {
            if( players[ i ].playerNumber == playerNumber ) {
                players[ i ].state = state;
                if( state == PLAYER_CONNECTION_STATE_SYNCHRONIZING ) {
                    players[ i ].connectProgress = arg1;
                }
                else if( state == PLAYER_CONNECTION_STATE_DISCONNECTING ) {
                    players[ i ].disconnectStart = arg1;
                    players[ i ].disconnectTimeout= arg2;
                }
            }
        }
    }

}

