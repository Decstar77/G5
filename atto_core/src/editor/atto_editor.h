#pragma once

#include "../shared/atto_defines.h"
#include "../game/modes/atto_game_mode_game.h"
#include "../game/sim/atto_sim_map.h"


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
        SimEntity *            entity = nullptr;
        SpriteResource *    spriteResource = nullptr;
    };

    struct Editor_CanvasWidget {
        i32 seletedTileIndex = -1;
        i32 flagValues = 0;
        bool showFlags = false;
    };

    class Editor {
    public:
        Editor();

        void        Initialize( Core * core );
        void        UpdateAndRender( Core * core, GameMode * game, f32 dt );
        void        MainMenuBar( Core * core );
        void        CanvasEditor( Core * core, GameMode * gameMode );
        void        ResourceEditor( Core * core, GameMode * gameMode );

        union {
            struct {
                bool canvas;
                bool resourceEditor;
            } show;
            bool windowShows[ sizeof( show ) / sizeof( bool ) ];
        };

        bool                    inGame = true;
        glm::vec2               cameraPos = glm::vec2( 0, 0 );
        f32                     cameraWidth = 640;
        f32                     cameraHeight = 360;
        bool                    isCameraDragging = false;
        glm::vec2               dragStartPos = glm::vec2( 0, 0 );
        glm::vec2               gizmoDrag = glm::vec2( 0, 0 );
        SimEntity *                gizmoEntity = nullptr;
        i32                     gizmoMode = 0;

        Editor_CanvasWidget     canvasWidget;
        Editor_ResourceWidget   resourceWidget;
        SimMap *                   currentMap = nullptr;

        TextureResource *       gizmoTranslationTexture = nullptr;
    };
}


#endif