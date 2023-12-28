#include "atto_game_mode_pixel.h"

namespace atto::pixy {
    GameModeType PixyGameMode::GetGameModeType() {
        return GameModeType::PIXY;
    }

    bool PixyGameMode::IsInitialized() {
        return true;
    }

    void PixyGameMode::Initialize( Core * core ) {
        
    }

    void PixyGameMode::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags /*= UPDATE_AND_RENDER_FLAG_NONE */ ) {
        DrawContext * worldDraws = core->RenderGetDrawContext( 0 );
        

        worldDraws->DrawRect( glm::vec2( 0 ), glm::vec2( 100 ) );
    }

    void PixyGameMode::Shutdown( Core * core ) {
        
    }

}