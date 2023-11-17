#pragma once
#include "../shared/atto_core.h"

namespace atto {
    enum class GameModeType {
        MAIN_MENU = 0,
        IN_GAME,
        OPTIONS,
    };

    class GameMode {
    public:
        virtual GameModeType       GetGameModeType() = 0;
        virtual void                Init( Core * core ) = 0;
        virtual void                Update( Core * core, f32 dt ) = 0;
        virtual void                Render( Core * core, f32 dt ) = 0;
        virtual void                Shutdown( Core * core ) = 0;
    };

    
    class Game {
    public:
        void Update( Core * core, f32 dt );
        void Render( Core * core, f32 dt );

    private:
        GameMode * gameMode = nullptr;
    };
}

