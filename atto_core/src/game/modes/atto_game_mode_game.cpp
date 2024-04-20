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
        simMap.localPlayerNumber.value = 1;
        simMap.localPlayerTeamNumber.value = 1;
        simMap.Initialize( core );
    }

    void GameMode_SinglePlayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags updateAndRenderFlags ) {
        simMap.Update( core, dt );
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
        if( startParms.localPlayerNumber == 1 ) {
            core->LogOutput( LogLevel::INFO, "I am the host" );
            isHost = true;
            simMap.localPlayerTeamNumber.value = 1;
        }
        else {
            core->LogOutput( LogLevel::INFO, "I am a peer" );
            isHost = false;
            simMap.localPlayerTeamNumber.value = 2;
        }

        simMap.localPlayerNumber.value = startParms.localPlayerNumber;

        simMap.Initialize( core );
    }

    void GameMode_MultiplayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        simMap.Update( core, dt );
    }

    void GameMode_MultiplayerGame::Shutdown( Core * core ) {

    }
}