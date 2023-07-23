
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

        if( core->InputKeyDown( KEY_CODE_A ) ) {
            sim->inputForNextSim = 1;
        }
        if( core->InputKeyDown( KEY_CODE_D ) ) {
            sim->inputForNextSim = 2;
        }
        if( core->InputKeyDown( KEY_CODE_W ) ) {
            sim->inputForNextSim = 3;
        }
        if( core->InputKeyDown( KEY_CODE_S ) ) {
            sim->inputForNextSim = 4;
        }

        core->RenderDrawText( arialFont, glm::vec2( 100, 100 ), debugText.GetCStr() );

        core->RenderDrawText( arialFont, glm::vec2( 100, 140 ),
            StringFormat::Small( "Ping = %d", core->NetGetPing() ).GetCStr() );


        TextureResource * res = core->ResourceGetAndLoadTexture( "unit_basic_man.png" );
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        p1VisPos = glm::mix( p1VisPos, sim->p1Pos, 0.1f );
        p2VisPos = glm::mix( p2VisPos, sim->p2Pos, 0.1f );

        Camera camera = core->RenderCreateCamera();
        core->RenderSetCamera( &camera );
        core->RenderDrawSprite( res, p1VisPos, glm::vec2( 1 ) );
        core->RenderDrawSprite( res, p2VisPos, glm::vec2( 1 ) );
        core->RenderSetCamera( nullptr );
        core->RenderSubmit();
    }

    void GameLogic::Shutdown( Core * core ) {
        
    }
}


