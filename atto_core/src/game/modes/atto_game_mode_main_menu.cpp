#include "atto_game_mode_main_menu.h"
#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

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
        static TextureResource * background = ResourceGetAndCreateTexture( "res/game/gui/main_menu_screen_01.png" );
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

        FontHandle fontHandle = core->ResourceGetFont( "default" );
        SmallString status = core->NetworkGetStatusText();
        draw->DrawTextScreen( fontHandle, glm::vec2( 128, 128 ), 32, status.GetCStr() );

        SmallString s = StringFormat::Small( "fps=%f", 1.0f / dt );
        draw->DrawTextScreen( fontHandle, glm::vec2( 528, 200 ), 32, s.GetCStr() );

        core->RenderSubmit( draw, true );
        UI_MainMenu( core );
    }

    void GameMode_MainMenu::Shutdown( Core * core ) {
    }
}