#include "atto_editor.h"
#include "../shared/atto_core.h"

#include "imgui.h"

namespace atto {

    Editor::Editor() {
    }

    void Editor::UpdateAndRender( Core * core, GameMode * game, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( game->IsInitialized() == false ) {
            game->Initialize( core );
            return;
        }

        game->UpdateAndRender( core, dt );
    }

    void Editor::MainMenuBar( Core * core, GameMode * game ) {
        GameMode_Game * leGame = ( (GameMode_Game *)game );
        if( ImGui::BeginMainMenuBar() ) {

            if( ImGui::MenuItem( "Canvas" ) ) {
                show.canvas = !show.canvas;
            }

            if( ImGui::MenuItem( "Save Map" ) ) {
                //leGame->map.DEBUG_SaveToFile( core, "res/maps/map.json" );
            }

            if( ImGui::MenuItem( "Load Map" ) ) {
                //leGame->map.DEBUG_LoadFromFile( core, "res/maps/map.json" );
            }

            ImGui::EndMainMenuBar();
        }
    }

}