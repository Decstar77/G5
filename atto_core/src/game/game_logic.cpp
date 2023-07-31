
#include "../shared/atto_core.h"

namespace atto {
    void GameLogic::Start( Core * core ) {
        AudioResource * audio = core->ResourceGetAndLoadAudio( "basic_death_1.wav" );
        core->AudioPlay( audio, 1.0f );

        arialFont = core->GetDebugFont();

        currentState = GameLocationState::MAIN_MENU;
    }

    void GameLogic::MainMenuUpdateAndRender( Core * core, SimLogic * sim ) {
        core->UIBegin();
        if( core->UIPushButton( "Single player", glm::vec2( 0.5f, 0.4f ), glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) ) ) {
            sim->StartSinglePlayerGame();
            currentState = GameLocationState::IN_GAME;
        }

        if( core->UIPushButton( "Multiplayer", glm::vec2( 0.5f, 0.5f ), glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) ) ) {
            sim->StartMultiplayerGame();
            currentState = GameLocationState::IN_GAME;
        }

        if( core->UIPushButton( "Map Editor", glm::vec2( 0.5f, 0.6f ), glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) ) ) {
        }

        if( core->UIPushButton( "Settings", glm::vec2( 0.5f, 0.7f ), glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) ) ) {
        }

        if( core->UIPushButton( "Quit", glm::vec2( 0.5f, 0.8f ), glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) ) ) {
        }

        core->UIEndAndRender();

        core->RenderSubmit();
    }

    void GameLogic::InGameUpdateAndRender( Core * core, SimLogic * sim ) {
        
        core->RenderDrawRect( glm::vec2( 0, 0 ), glm::vec2( 1280, 720 ), glm::vec4( 0.3f, 0.6f, 0.3f, 1 ) );

        MultiplayerState * mp = core->GetMPState();

        TextureResource * blueTank = core->ResourceGetAndLoadTexture( "tank_set/tankBody_blue.png" );
        TextureResource * blueTankTurret = core->ResourceGetAndLoadTexture( "tank_set/tankBlue_barrel1.png" );
        TextureResource * redTank = core->ResourceGetAndLoadTexture( "tank_set/tankBody_red.png" );
        TextureResource * metalWall = core->ResourceGetAndLoadTexture( "metal_elements/elementMetal011.png" );
        
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        f32 p1TurretRot = FixedToFloat( sim->state.playerTanks[ 0 ].turretRot );
        glm::vec2 p1Pos = FpVec2ToVec2( sim->state.playerTanks[ 0 ].pos );
        f32 p2TurretRot = FixedToFloat( sim->state.playerTanks[ 1 ].turretRot );
        glm::vec2 p2Pos = FpVec2ToVec2( sim->state.playerTanks[ 1 ].pos );

        p1VisTurretRot = glm::mix( p1VisTurretRot, p1TurretRot, 0.1f );
        p1VisPos = glm::mix( p1VisPos, p1Pos, 0.1f );

        p2VisPos = glm::mix( p2VisPos, p2Pos, 0.1f );

        glm::vec2 mousePosPix = core->InputMousePosPixels();
        //core->LogOutput( LogLevel::DEBUG, "Mouse pos pix %f %f", mousePosPix.x, mousePosPix.y );
        glm::vec2 mousePosWorld = core->ScreenPosToWorldPos( mousePosPix );
        core->LogOutput( LogLevel::DEBUG, "Mouse pos world %f %f", mousePosWorld.x, mousePosWorld.y );

        core->RenderDrawRect( mousePosWorld, mousePosWorld + glm::vec2( 25 ) );

        core->RenderDrawSprite( blueTank, p1VisPos );
        core->RenderDrawSprite( blueTankTurret, p1VisPos, p1TurretRot );

        core->RenderDrawSprite( redTank, p2VisPos );

        const i32 mapElementCount = sim->state.elements.GetCount();
        for( i32 i = 0; i < mapElementCount; i++ ) {
            const MapElement & element = sim->state.elements[ i ];
            glm::vec2 pos = FpVec2ToVec2(element.pos);
            core->RenderDrawSpriteBL( metalWall, pos, glm::vec2( 0.5f ) );
        }

        core->RenderDrawText( arialFont, glm::vec2( 50, 100 ),
            StringFormat::Small( "Player number %d, player handle %d", mp->local.playerNumber, mp->local.playerHandle ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 130 ),
            StringFormat::Small( "Local state %s", PlayerConnectStateToString( mp->local.state ).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 400, 130 ),
            StringFormat::Small( "Peer state %s", PlayerConnectStateToString( mp->peer.state ).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 190 ),
            StringFormat::Small( "Ping To Server = %d", core->NetGetPing() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 220 ),
            StringFormat::Small( "Ping To Peers = %d", core->GetMPState()->peer.pingToPeer ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 250 ),
            StringFormat::Small( "KBS= %d", core->GetMPState()->peer.kbsSent ).GetCStr() );

        core->RenderSubmit();
    }

    void GameLogic::UpdateAndRender( Core * core, SimLogic * sim ) {
        f32 dt = core->GetDeltaTime();

        if( core->InputKeyJustPressed( KEY_CODE_F1 ) ) {
            core->NetConnect();
        }

        if( core->InputKeyJustPressed( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

#if 1
        switch( currentState ) {
            case atto::GameLocationState::NONE:
            {
                INVALID_CODE_PATH;
            } break;
            case atto::GameLocationState::MAIN_MENU:
            {
                MainMenuUpdateAndRender( core, sim );
            } break;
            case atto::GameLocationState::IN_GAME:
            {
                InGameUpdateAndRender( core, sim );
            } break;
            case atto::GameLocationState::OPTIONS:
            {
            } break;
            default:
            {
                INVALID_CODE_PATH;
            }
        }
#else
        MultiplayerState * mp = core->GetMPState();

        core->RenderDrawText( arialFont, glm::vec2( 50, 100 ),
            StringFormat::Small( "Player number %d, player handle %d", mp->local.playerNumber, mp->local.playerHandle ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 130 ),
            StringFormat::Small( "Local state %s", PlayerConnectStateToString(mp->local.state).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 400, 130 ),
            StringFormat::Small( "Peer state %s", PlayerConnectStateToString( mp->peer.state ).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 190 ),
            StringFormat::Small( "Ping To Server = %d", core->NetGetPing() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 50, 220 ),
            StringFormat::Small( "Ping To Peers = %d", core->GetMPState()->peer.pingToPeer ).GetCStr());

        core->RenderDrawText( arialFont, glm::vec2( 50, 250 ),
            StringFormat::Small( "KBS= %d", core->GetMPState()->peer.kbsSent ).GetCStr() );

        TextureResource * res = core->ResourceGetAndLoadTexture( "tank_set/tankBody_blue_outline.png" );
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        p1VisPos = glm::mix( p1VisPos, sim->state.p1Pos, 0.1f );
        p2VisPos = glm::mix( p2VisPos, sim->state.p2Pos, 0.1f );

        Camera camera = core->RenderCreateCamera();
        core->RenderSetCamera( &camera );
        core->RenderDrawCircle( p1VisPos, 10.0f, glm::vec4( 1, 0, 0, 1 ) );
        core->RenderDrawCircle( p2VisPos, 10.0f, glm::vec4( 0, 1, 0, 1 ) );
        core->RenderDrawSprite( res, glm::vec2( 0, 0 ) );
        core->RenderSetCamera( nullptr );
        core->RenderSubmit();
#endif
    }

    void GameLogic::Shutdown( Core * core ) {
        
    }
}


