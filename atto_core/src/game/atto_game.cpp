#include "atto_game.h"
#include "modes/atto_game_mode_game.h"

namespace atto {
    void Game::UpdateAndRender( Core * core, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( gameMode == nullptr ) {
            gameMode = new GameModeGame();
            gameMode->Init( core );
        }

        gameMode->UpdateAndRender( core, dt );
    }
}
