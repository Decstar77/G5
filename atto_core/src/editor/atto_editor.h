#pragma once

#include "../shared/atto_defines.h"
#include "../game/modes/atto_game_mode_game.h"

#if ATTO_EDITOR

struct GLFWwindow;

namespace atto {

    class EngineImgui {
    public:
        static void Initialize( GLFWwindow * window );
        static void Shutdown();

        static void NewFrame();
        static void EndFrame();
    };

    class Editor {
    public:
        Editor();

        void        UpdateAndRender( Core * core, GameMode * game, f32 dt );
        void        MainMenuBar( Core * core );
        
        bool            editorActive = false;
        i32             updateAndRenderFlags = UPDATE_AND_RENDER_FLAG_NONE; // UpdateAndRenderFlags

        union {
            struct {
                bool canvas;
            } show;
            bool windowShows[ sizeof( show ) / sizeof( bool ) ];
        };
        
    };
}


#endif