#include "atto_game.h"
#include "modes/atto_game_mode_game.h"

namespace atto {
    void Game::Update( Core * core, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( gameMode == nullptr ) {
            gameMode = new GameModeGame();
            gameMode->Init( core );
        }

        gameMode->Update( core, dt );
    }

    void Game::Render( Core * core, f32 dt ) {
        gameMode->Render( core, dt );
    }
}
