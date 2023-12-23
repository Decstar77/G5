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
        static void DoFrame();
        static void EndFrame();
    };

    struct EditorCamera {
        glm::vec3 pos;
        EntCamera camera;

        inline glm::mat4 GetViewMatrix() const { return glm::lookAt( pos, pos + camera.front, camera.up ); }
    };

    class Editor {
    public:
        Editor();
        
        void UpdateAndRender( Core * core, GameMode * game, f32 dt );
        void MainMenuBar( Core * core, GameMode * game );
        void Canvas( Core * core, GameMode * game );
        void UpdateEditorCamera( Core * core );
        
        bool            editorActive = false;
        i32             updateAndRenderFlags = UPDATE_AND_RENDER_FLAG_NONE; // UpdateAndRenderFlags
        EditorCamera    editorCamera = {};

        union {
            struct {
                bool canvas;
            } show;
            bool windowShows[ sizeof( show ) / sizeof( bool ) ];
        };
        
    };
}


#endif