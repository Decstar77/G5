#include "atto_game_mode_main_menu.h"
#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

// @TODO: REMOVE THIS.
#include "../../content/atto_content.h"

namespace atto {
    GameModeType GameMode_MainMenu::GetGameModeType() {
        return GameModeType::MAIN_MENU;
    }

    bool GameMode_MainMenu::IsInitialized() {
        return true;
    }

    void GameMode_MainMenu::Initialize( Core * core ) {
    }

    void GameMode_MainMenu::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags /*= UPDATE_AND_RENDER_FLAG_NONE */ ) {
        static TextureResource * background = core->ResourceGetAndCreateTexture( "res/sprites/ui/main_menu_screen_01.png", false, false );
        DrawContext * draw = core->RenderGetDrawContext( 0, true );
        draw->DrawTextureBL( background, glm::vec2( 0, 0 ) );

        if( core->NetworkIsConnected() ) {
            NetworkMessage msg = {};
            while( core->NetworkRecieve( msg ) ) {
                switch( msg.type ) {
                    case NetworkMessageType::NONE:
                        INVALID_CODE_PATH;
                        break;
                    case NetworkMessageType::GAME_START:
                    {
                        i32 offset = 0;
                        i32 localPlayerNumber = NetworkMessagePop<i32>( msg, offset );
                        i32 otherPlayerNumber = NetworkMessagePop<i32>( msg, offset );

                        GameStartParams parms = {};
                        parms.isMutliplayer = true;
                        parms.localPlayerNumber = localPlayerNumber;
                        parms.otherPlayerNumber = otherPlayerNumber;

                        core->MoveToGameMode( new GameMode_MultiplayerGame( parms ) );
                    } break;
                    default:
                        INVALID_CODE_PATH;
                        break;
                }
            }
        }

        glm::vec2 c = draw->GetCameraDims() / 2.0f;

        UIWidgetSize textSize = {};
        textSize.type = UI_SizeType::TEXTCONTENT;
        UIWidgetSize childSum = {};
        childSum.type = UI_SizeType::CHILDRENSUM;
        UIWidgetSize childMax = {};
        childMax.type = UI_SizeType::CHILDRENMAX;
        UIWidgetPos  centerPos = {};
        centerPos.type = UI_PosType::CENTER;
        UIWidgetSize paretPercent = {};
        paretPercent.type = UI_SizeType::PERCENTOFPARENT;
        paretPercent.value = 1.0f;

        ui.Begin( draw->GetCameraDims() );
        ui.BeginVBox( 1, centerPos, childMax, childSum, Colors::MIDNIGHT_BLUE );
        ui.Button( 22, "The game name is here", centerPos, textSize, textSize, Colors::ALIZARIN );

        ui.BeginVBox( 2, centerPos, paretPercent, childSum, Colors::SKY_BLUE );
        //ui.Button( 33, "Tutorial", centerPos, paretPercent, textSize , Colors::ALIZARIN );
        if ( ui.Button( 44, "Single Player",  centerPos, paretPercent, textSize, Colors::ALIZARIN ) ) {
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_SinglePlayerGame( parms ) );
        }
        if ( ui.Button( 55, "Mutliplayer",  centerPos, paretPercent, textSize, Colors::ALIZARIN ) ) {
            core->NetConnect();
        }
        if ( ui.Button( 66, "Quit", centerPos, paretPercent, textSize, Colors::ALIZARIN ) ) {
            core->WindowClose();
        }
        ui.EndVBox();

        ui.EndVBox();
        ui.UpdateAndRender( core, draw );
        ui.End();

//        ui.BeginBox( core->RenderGetMainSurfaceWidth() / 2, core->RenderGetMainSurfaceHeight() - 250, true );
//        if( ui.AddButton( "Loner" ) ) {
//            GameStartParams parms = {};
//            parms.isMutliplayer = false;
//            core->MoveToGameMode( new GameMode_SinglePlayerGame( parms ) );
//        }
//        if( ui.AddButton( "Blessed" ) ) {
//            core->NetConnect();
//        }
//        if( ui.AddButton( "Soon" ) ) {
//            core->WindowClose();
//        }
//        ui.EndBox();
//        ui.Draw( draw );

        FontHandle fontHandle = core->ResourceGetFont( "default" );
        SmallString status = core->NetworkGetStatusText();
        draw->DrawTextScreen( fontHandle, glm::vec2( 128, 128 ), 32, status.GetCStr() );

        //static PixelArtMeshCreator pixyMesh;
        //if ( pixyMesh.IsLoaded() == false ) {
        //    pixyMesh.LoadFromFile( "res/ents/test/ship_blue.png" );
        //}

        //glm::vec2 offset = glm::vec2( 100 );

        //static f32 rot = 0.0f;
        //rot += dt;

        //f32 scale = 0.06f;

        //for ( size_t i = 0; i < pixyMesh.points.size(); i++ ) {
        //    auto pp = pixyMesh.points[i];
        //    glm::vec2 p = pp.p;
        //    p = glm::rotate( p, -rot ) * scale;
        //    draw->DrawRect( offset + p, scale * glm::vec2( 10.0f ), rot, pp.c );
        //}

        //static TextureResource * sprShip = core->ResourceGetAndLoadTexture( "res/ents/test/ship_blue.png" );
        //draw->DrawTexture( sprShip, offset + glm::vec2( 64, 0 ), rot, glm::vec2( 0.6f ) );

        SmallString s = StringFormat::Small( "fps=%f", 1.0f / dt );
        draw->DrawTextScreen( fontHandle, glm::vec2( 528, 200 ), 32, s.GetCStr() );

        core->RenderSubmit( draw, true );
    }

    void GameMode_MainMenu::Shutdown( Core * core ) {
    }
}