#include "atto_editor.h"
#include "../shared/atto_core.h"
#include "../game/atto_game.h"
#include "../game/modes/atto_game_mode_game.h"
#include "imgui.h"

namespace atto {
    void Editor::UpdateAndRender( Core * core, Game * game, f32 dt ) {
        //ImGui::ShowDemoWindow();
        game->UpdateAndRender( core, dt );
    }
}