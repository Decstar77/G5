#include "atto_ggpo.h"
#include "atto_core.h"


namespace atto {

    static Core * theCore = nullptr;
    static bool BeginGame( const char * game ) {
        return true;
    }

    static bool AdvanceFrame( int flags ) {
        theCore->GetSimLogic()->Advance();
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
                theCore->GetSimLogic()->SetConnectionState( info->u.connected.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, 0, 0 );
            } break;
            case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
            {
                int p = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
                theCore->GetSimLogic()->SetConnectionState( info->u.synchronizing.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, p, 0 );
            }break;
            case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
            {
                theCore->GetSimLogic()->SetConnectionState( info->u.synchronized.player, PLAYER_CONNECTION_STATE_SYNCHRONIZING, 100, 0 );
            } break;
            case GGPO_EVENTCODE_RUNNING:
            {
                theCore->GetSimLogic()->SetConnectionState( PLAYER_CONNECTION_STATE_RUNNING, 0, 0 );
            } break;
            case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
            {
                int n = (int)theCore->GetLastTime();
                int d = info->u.connection_interrupted.disconnect_timeout;
                theCore->GetSimLogic()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_DISCONNECTING, n, d );
            } break;
            case GGPO_EVENTCODE_CONNECTION_RESUMED:
            {
                theCore->GetSimLogic()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_RUNNING, 0, 0 );
            }
            break;
            case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
            {
                theCore->GetSimLogic()->SetConnectionState( info->u.connection_interrupted.player, PLAYER_CONNECTION_STATE_DISCONNECTED, 0, 0 );
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
        //theCore->GetNetClient()->Send()
    }

    static bool NetworkPoll( GPPONetworkCallbacks * msg ) {
        //theCore->GetNetClient()->Recieve();
        return true;
    }

    void Core::GGPOInitializeInstance() {
        theCore = this;

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
    }
}

