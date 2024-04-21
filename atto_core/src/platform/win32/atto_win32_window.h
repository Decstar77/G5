#pragma once

#include "../../shared/atto_containers.h"

struct GLFWmonitor;
struct GLFWwindow;

namespace atto {

    class Core;

    class PlatformWindow {
    public:
        void    Initialize( Core * core );
        f64     GetTime() const;
        bool    ShouldClose();
        void    PollEvents();
        void    SwapBuffers();
        void    SetCursorDisable();
        void    SetCursorNormal();
        bool    IsCursorDisabled();
        void    SetWindowTitle( const char * title );
        void    Close();
        void    GetFramebufferSize( i32 & w, i32 & h );
        void    SetVysnc( bool v );
        void    EnableFullscreen();
        void    DisableFullscreen();

        GLFWwindow * GetWindowPtr() { return window; }

    public:
        Core *                      core = nullptr;
        GLFWmonitor *               monitor = nullptr;
        LargeString                 monitorName = LargeString::FromLiteral( "" );
        f64                         monitorRefreshRate = 0;

        GLFWwindow *                window = nullptr;
        SmallString                 windowTitle = SmallString::FromLiteral( "Game" );
        bool                        windowFullscreen = false;
    };

}
