#pragma once

#include "../atto_game.h"
#include "../../shared/atto_ui.h"

namespace atto {
    
    class GameMode_MainMenu : public GameMode {

    public:
        GameModeType GetGameModeType() override;
        bool IsInitialized() override;
        void Initialize( Core * core ) override;
        void UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags = UPDATE_AND_RENDER_FLAG_NONE ) override;
        void Shutdown( Core * core ) override;

        UIContext ui;
    };
    
}