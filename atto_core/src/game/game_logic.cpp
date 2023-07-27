
#include "../shared/atto_core.h"

namespace atto {
    void GameLogic::Start( Core * core ) {
        AudioResource * audio = core->ResourceGetAndLoadAudio( "basic_death_1.wav" );
        core->AudioPlay( audio, 1.0f );

        arialFont = core->ResourceGetAndLoadFont( "arial.ttf", 24 );
        debugText = "Hello ?";
    }

    void GameLogic::UpdateAndRender( Core * core, SimLogic * sim ) {
        f32 dt = core->GetDeltaTime();

        if( core->InputKeyJustPressed( KEY_CODE_F1 ) ) {
            core->NetConnect();
        }

        if( core->InputKeyJustPressed( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        MultiplayerState * mp = core->GetMPState();

        core->RenderDrawText( arialFont, glm::vec2( 100, 100 ), 
            StringFormat::Small( "Player number %d, player handle %d", mp->local.playerNumber, mp->local.playerHandle ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 100, 130 ),
            StringFormat::Small( "Local state %s", PlayerConnectStateToString(mp->local.state).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 400, 130 ),
            StringFormat::Small( "Peer state %s", PlayerConnectStateToString( mp->peer.state ).GetCStr() ).GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 100, 190 ),
            StringFormat::Small( "Ping = %d", core->NetGetPing() ).GetCStr() );


        TextureResource * res = core->ResourceGetAndLoadTexture( "unit_basic_man.png" );
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        p1VisPos = glm::mix( p1VisPos, sim->state.p1Pos, 0.1f );
        p2VisPos = glm::mix( p2VisPos, sim->state.p2Pos, 0.1f );

        Camera camera = core->RenderCreateCamera();
        core->RenderSetCamera( &camera );
        core->RenderDrawCircle( p1VisPos, 10.0f, glm::vec4( 1, 0, 0, 1 ) );
        core->RenderDrawCircle( p2VisPos, 10.0f, glm::vec4( 0, 1, 0, 1 ) );
        core->RenderSetCamera( nullptr );
        core->RenderSubmit();
    }

    void GameLogic::Shutdown( Core * core ) {
        
    }
}


