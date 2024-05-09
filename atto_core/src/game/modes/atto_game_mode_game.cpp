#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {

    GameModeType GameMode_SinglePlayerGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_SinglePlayerGame::IsInitialized() {
        return true;
    }

    void GameMode_SinglePlayerGame::Initialize( Core * core ) {
        visMap.playerNumber.value = 1;
        visMap.teamNumber.value = 1;
        visMap.SimInitialize();
        visMap.VisInitialize( core );

        visMap.SimNextTurn( nullptr, nullptr, tickCount );
        turnNumber++;
    }

    void GameMode_SinglePlayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags updateAndRenderFlags ) {
        if ( visMap.SimDoneTicks() == true ) {
            MapTurn player1Turn = {};
            player1Turn.checkSum = 1;
            player1Turn.turnNumber = turnNumber + 1;
            player1Turn.actions = visMap.actionBuffer;
            ZeroStruct( visMap.actionBuffer );

            visMap.SimNextTurn( &player1Turn, nullptr, tickCount );
            turnNumber++;
        }

        visMap.SimNextTick( dt );
        visMap.VisUpdate( dt );
    }

    void GameMode_SinglePlayerGame::Shutdown( Core * core ) {
    }

    GameModeType GameMode_MultiplayerGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_MultiplayerGame::IsInitialized() {
        return true;
    }

    void GameMode_MultiplayerGame::Initialize( Core * core ) {
        visMap.playerNumber.value = startParms.localPlayerNumber;
        visMap.teamNumber.value = startParms.localPlayerNumber;
        visMap.SimInitialize();
        visMap.VisInitialize( core );
        
        visMap.SimNextTurn( nullptr, nullptr, tickCount );
        turnNumber++;

        ZeroStruct( localTurn );
        localTurn.checkSum = 2;
        localTurn.turnNumber = turnNumber + 1;

        NetworkMessage & msg = *MemoryAllocateTransient < NetworkMessage > ();
        ZeroStruct( msg );
        msg.type = NetworkMessageType::MAP_TURN;
        NetworkMessagePush( msg, localTurn );
        NetworkSend( msg );
    }

    void GameMode_MultiplayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        static f32 echoTimer = 0.0f;
        static bool recEcho = true;
        echoTimer += dt;
        
        NetworkMessage & msg = *MemoryAllocateTransient < NetworkMessage > ();

        if ( echoTimer > 1.0f && recEcho == true ) {
            recEcho = false;
            msg.type = NetworkMessageType::ECHO;
            NetworkMessagePush<i32>( msg, visMap.playerNumber.value );
            NetworkMessagePush<f32>( msg, echoTimer );
            //NetworkSend( msg );
        }

        while ( NetworkRecieve( msg ) ) {
            if ( msg.type == NetworkMessageType::MAP_TURN ) {
                i32 offset = 0;
                MapTurn turn = NetworkMessagePop<MapTurn>( msg, offset );
                //ATTOINFO( "Rec turn %d", turn.turnNumber );
                peerTurns.Enqueue( turn );
            } else if ( msg.type == NetworkMessageType::ECHO ) {
                i32 offset = 0;
                i32 id = NetworkMessagePop<i32>( msg, offset );
                if ( id == visMap.playerNumber.value ) {
                    f32 time = NetworkMessagePop<f32>( msg, offset );
                    f32 delta = ( echoTimer - time ) * 1000.0f;
                    ATTOINFO( "Ping time %f", ( f32 )delta  );
                    recEcho = true;
                } else {
                    NetworkSend( msg );
                }
            }
        }

        if ( visMap.SimDoneTicks() == true ) {
            if ( peerTurns.IsEmpty() == false ) {
                MapTurn peerTurn = peerTurns.Dequeue();
                Assert( localTurn.turnNumber == peerTurn.turnNumber );
                Assert( localTurn.checkSum == peerTurn.checkSum );
                visMap.SimNextTurn( &localTurn, &peerTurn, tickCount );
                turnNumber++;
                //ATTOINFO( "Turn %d", turnNumber );

                ZeroStruct( peerTurn );
                ZeroStruct( localTurn );
                localTurn.checkSum = 1;
                localTurn.turnNumber = turnNumber + 1;
                localTurn.actions = visMap.actionBuffer;
                ZeroStruct( visMap.actionBuffer );

                //ATTOINFO( "Snd %d", localTurn.turnNumber );
                ZeroStruct( msg );
                msg.type = NetworkMessageType::MAP_TURN;
                NetworkMessagePush( msg, localTurn );
                NetworkSend( msg );
            } else {
                ATTOWARN( "OH BOYYYYOOOO" );
            }
        }

        visMap.SimNextTick( dt );
        visMap.VisUpdate( dt );
    }

    void GameMode_MultiplayerGame::Shutdown( Core * core ) {
        
    }
}