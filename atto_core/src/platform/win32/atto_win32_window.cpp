#include "atto_win32_window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../../game/atto_core.h"

namespace atto {
    
    static bool                         firstMouse = true;
    static void KeyCallback( GLFWwindow * window, int key, int scancode, int action, int mods ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        if( key == KEY_CODE_UNKNOWN ) {
            return;
        }
        fi.keys[ key ] = action != GLFW_RELEASE;
    }

    static void MousePositionCallback( GLFWwindow * window, double xpos, double ypos ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        if( firstMouse ) {
            fi.lastMousePosPixels.x = (f32)xpos;
            fi.lastMousePosPixels.y = (f32)ypos;
            firstMouse = false;
        }
        else {
            fi.lastMousePosPixels = fi.mousePosPixels;
        }

        fi.mousePosPixels = glm::vec2( (f32)xpos, (f32)ypos );
        // We need to += here because this callback can be called multiple times when calling glfwPollEvents. So we need to collect all the inputs.
        fi.mouseDeltaPixels.x += fi.mousePosPixels.x - fi.lastMousePosPixels.x;
        fi.mouseDeltaPixels.y += fi.lastMousePosPixels.y - fi.mousePosPixels.y;
    }

    static void MouseButtonCallback( GLFWwindow * window, int button, int action, int mods ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        fi.mouseButtons[ button ] = action != GLFW_RELEASE;

        core->NuklearUIMouseButton( button, action, mods );
    }

    static void ScrollCallback( GLFWwindow * window, double xoffset, double yoffset ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        fi.mouseWheelDelta = glm::vec2( (f32)xoffset, (f32)yoffset );
        core->NuklearUIScroll( xoffset, yoffset );
    }

    static void FramebufferCallback( GLFWwindow * window, i32 w, i32 h ) {
    }

    static void CharCallback( GLFWwindow * window, u32 codepoint ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        core->NuklearUIChar( codepoint );
    }

    void PlatformWindow::Initialize( Core * core ) {
        if( !glfwInit() ) {
            ATTOFATAL( "Could not init glfw, your windows is :( " );
            return;
        }

        this->core = core;

        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );

    #if ATTO_DEBUG_RENDERING && ATTO_OPENGL
        glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
    #endif
        
    #if ATTO_OPENGL
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    #elif ATTO_VULKAN
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    #endif

        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
        glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );
        //glfwWindowHint( GLFW_SAMPLES, 4 );

        monitor = glfwGetPrimaryMonitor();
        if( monitor != nullptr ) {
            const GLFWvidmode * videoMode = glfwGetVideoMode( monitor );
            monitorRefreshRate = videoMode->refreshRate;
            monitorName = glfwGetMonitorName( monitor );
            //ATTOINFO("Using monitor name %s", os.monitorName.GetCStr());
        }
        else {
            ATTOFATAL( "Could not find a monitor to render on." );
            return;
        }

        GameSettings gameSettings = core->GetCurrentGameSettings();

        i32 windowWidth = gameSettings.windowWidth;
        i32 windowHeight = gameSettings.windowHeight;
        window = glfwCreateWindow( windowWidth, windowHeight, windowTitle.GetCStr(), nullptr, 0 );

        if( window == nullptr ) {
            ATTOFATAL( "Could not create window, your windows is :( " );
            return;
        }

        glfwMakeContextCurrent( window );
        glfwSwapInterval( gameSettings.vsync );

        glfwSetWindowUserPointer( window, core );

        if( glfwRawMouseMotionSupported() ) {
            glfwSetInputMode( window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
        }
        else {
            ATTOWARN( "Could not enable raw input" );
        }

        glfwSetCursorPosCallback( window, MousePositionCallback );
        glfwSetKeyCallback( window, KeyCallback );
        glfwSetMouseButtonCallback( window, MouseButtonCallback );
        glfwSetScrollCallback( window, ScrollCallback );
        glfwSetFramebufferSizeCallback( window, FramebufferCallback );
        glfwSetCharCallback( window, CharCallback );

        if ( gameSettings.fullscreen == false ) {
            if ( gameSettings.windowStartPosX != -1 ) {
                glfwSetWindowPos( window, gameSettings.windowStartPosX, gameSettings.windowStartPosY );
            }
            else {
                const GLFWvidmode * mode = glfwGetVideoMode( monitor );
                i32 monitorWidth = mode->width;
                i32 monitorHeight = mode->height;
                i32 windowPosX = ( monitorWidth - windowWidth ) / 2;
                i32 windowPosY = ( monitorHeight - windowHeight ) / 2;
                glfwSetWindowPos( window, windowPosX, windowPosY );
            }
        } else {
            EnableFullscreen();
        }
    }

    f64 PlatformWindow::GetTime() const {
        return glfwGetTime();
    }

    bool PlatformWindow::ShouldClose() {
        return glfwWindowShouldClose( window );
    }

    void PlatformWindow::PollEvents() {
        glfwPollEvents();
    }

    void PlatformWindow::SwapBuffers() {
        glfwSwapBuffers( window );
    }

    void PlatformWindow::SetCursorDisable() {
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    }

    void PlatformWindow::SetCursorNormal() {
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    }

    bool PlatformWindow::IsCursorDisabled() {
        return glfwGetInputMode( window, GLFW_CURSOR ) == GLFW_CURSOR_DISABLED;
    }

    void PlatformWindow::SetWindowTitle( const char * title ) {
        glfwSetWindowTitle( window, title );
    }

    void PlatformWindow::Close() {
        glfwSetWindowShouldClose( window, true );
    }

    void PlatformWindow::GetFramebufferSize( i32 & w, i32 & h ) {
        glfwGetFramebufferSize( window, &w, &h );
    }

    void PlatformWindow::SetVysnc( bool v ) {
        glfwSwapInterval( (i32)v );
    }

    void PlatformWindow::EnableFullscreen() {
        const GLFWvidmode * mode = glfwGetVideoMode( monitor );
        glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
        windowFullscreen = true;
    }

    void PlatformWindow::DisableFullscreen() {
        GameSettings gameSettings = core->GetCurrentGameSettings();
        const GLFWvidmode * mode = glfwGetVideoMode( monitor );
        i32 monitorWidth = mode->width;
        i32 monitorHeight = mode->height;
        i32 windowPosX = ( monitorWidth - gameSettings.windowWidth ) / 2;
        i32 windowPosY = ( monitorHeight - gameSettings.windowHeight ) / 2;
        glfwSetWindowMonitor( window, nullptr, windowPosX, windowPosY, gameSettings.windowWidth, gameSettings.windowHeight, 0 );
        windowFullscreen = false;
    }
    
    f64 PlatformGetCurrentTime() {
        return glfwGetTime();
    }
}