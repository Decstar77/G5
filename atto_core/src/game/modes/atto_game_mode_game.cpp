#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    void SimCommunicator_SinglePlayer::SendToAllClients( MapActionBuffer * actionBuffer ) {
        visMap->ApplyActions( actionBuffer );
    }

    GameModeType GameMode_SinglePlayerGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_SinglePlayerGame::IsInitialized() {
        return true;
    }

    void GameMode_SinglePlayerGame::Initialize( Core * core ) {
        communicator.visMap = &visMap;
        visMap.playerNumber.value = 1;
        visMap.SimInitialize();
        visMap.VisInitialize( core );
    }

    void GameMode_SinglePlayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags updateAndRenderFlags ) {
        visMap.ApplyActions( &visMap.visActionBuffer );
        visMap.ApplyActions( &visMap.simActionBuffer );
        visMap.SimUpdate( dt );
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
    }

    void GameMode_MultiplayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        if ( core->NetworkIsConnected() == true ) {
            NetworkMessage & msg = *MemoryAllocateTransient < NetworkMessage > ();
            while ( core->NetworkRecieve( msg ) ) {
                switch ( msg.type ) {
                    case NetworkMessageType::ACTION_BUFFER:
                    {
                        i32 offset = 0;
                        MapActionBuffer actionBuffer = NetworkMessagePop<MapActionBuffer>( msg, offset );
                        visMap.ApplyActions( &actionBuffer );
                    } break;
                    case NetworkMessageType::STREAM_DATA: {
                        i32 offset = 0;
                        i32 count = NetworkMessagePop<i32>( msg, offset );
                        i32 streamCounter = NetworkMessagePop<i32>( msg, offset );
                        if ( streamCounter > visStreamDataCounter ) {
                            visStreamDataCounter = streamCounter;
                            for ( i32 i = 0; i < count; i++ ) {
                                SimStreamData data = NetworkMessagePop<SimStreamData>( msg, offset );
                                SimEntity * ent = visMap.entityPool.Get( data.handle );
                                if ( ent != nullptr ) {
                                    //ent->lastPos = ent->pos;
                                    //ent->lastPosTimer = 0.0f;
                                    //ent->pos = data.pos;
                                    ent->posTimeline.AddFrame( data.pos );
                                }
                            }
                        }

                    } break;
                }
            }
        }

        visMap.VisUpdate( dt );

        if ( core->NetworkIsConnected() == true && visMap.visActionBuffer.data.GetSize() != 0 ) {
            NetworkMessage & msg = *MemoryAllocateTransient < NetworkMessage > ();
            ZeroStruct( msg );
            msg.type = NetworkMessageType::ACTION_BUFFER;
            NetworkMessagePush( msg, visMap.visActionBuffer );
            core->NetworkSend( msg );
        }
    }

    void GameMode_MultiplayerGame::Shutdown( Core * core ) {
        
    }
}