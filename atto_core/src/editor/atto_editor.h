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

    struct Editor_ResourceWidget {
        Entity *            entity;
        SpriteResource *    spriteResource;
    };

    class Editor {
    public:
        Editor();

        void        UpdateAndRender( Core * core, GameMode * game, f32 dt );
        void        MainMenuBar( Core * core );
        void        ResourceEditor( Core * core, GameMode * gameMode );

        union {
            struct {
                bool canvas;
                bool resourceEditor;
            } show;
            bool windowShows[ sizeof( show ) / sizeof( bool ) ];
        };

        Editor_ResourceWidget resourceWidget;
        

        Map *                 currentMap = nullptr;
    };
}


#endif