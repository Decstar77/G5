#include "atto_game_mode_main_menu.h"
#include "atto_game_mode_game.h"

namespace atto {
    GameModeType GameMode_MainMenu::GetGameModeType() {
        return GameModeType::MAIN_MENU;
    }

    bool GameMode_MainMenu::IsInitialized() {
        return true;
    }

    void GameMode_MainMenu::Initialize( Core * core ) {
        ui.Initialize( core );
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


        ui.BeginBox( core->RenderGetMainSurfaceWidth() / 2, core->RenderGetMainSurfaceHeight() - 250, true );
        if( ui.AddButton( "Loner" ) ) {
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_SinglePlayerGame( parms ) );
        }
        if( ui.AddButton( "Blessed" ) ) {
            core->NetConnect();
        }
        if( ui.AddButton( "Soon" ) ) {
            core->WindowClose();
        }
        ui.EndBox();
        ui.Draw( draw );

        FontHandle fontHandle = core->ResourceGetFont( "default" );
        SmallString status = core->NetworkGetStatusText();
        draw->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, status.GetCStr() );

        core->RenderSubmit( draw, true );
    }

    void GameMode_MainMenu::Shutdown( Core * core ) {
    }
}