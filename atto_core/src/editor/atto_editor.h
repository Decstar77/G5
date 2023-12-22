#pragma once

#include "../shared/atto_defines.h"

#if ATTO_EDITOR

struct GLFWwindow;

namespace atto {
    class EngineImgui {
    public:
        static void Initialize( GLFWwindow * window );
        static void Shutdown();

        static void NewFrame();
        static void DoFrame();
        static void EndFrame();
    };

    class Core;
    class Game;
    class Editor {
    public:
        void UpdateAndRender( Core * core, Game * game, f32 dt );
        void MainMenuBar( Core * core, Game * game );
        void Canvas( Core * core, Game * game );

        union {
            struct {
                bool canvas;
            } show;
            bool windowShows[ sizeof( show ) / sizeof( bool ) ];
        };
        
    };
}


#endif