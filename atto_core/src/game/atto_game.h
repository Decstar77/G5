#pragma once
#include "../shared/atto_core.h"

namespace atto {
    enum class GameModeType {
        MAIN_MENU = 0,
        IN_GAME,
        OPTIONS,
    };

    enum UpdateAndRenderFlags {
        UPDATE_AND_RENDER_FLAG_NONE = 0,
        UPDATE_AND_RENDER_FLAG_NO_UPDATE = SetABit( 1 ),
        UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER = SetABit( 2 ),
    };

    class Map;
    class GameMode {
    public:
        virtual GameModeType        GetGameModeType() = 0;
        virtual bool                IsInitialized() = 0;
        virtual void                Initialize( Core * core ) = 0;
        virtual void                UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags = UPDATE_AND_RENDER_FLAG_NONE ) = 0;
        virtual void                Shutdown( Core * core ) = 0;
        virtual Map *               GetMap() = 0;
    };
}

