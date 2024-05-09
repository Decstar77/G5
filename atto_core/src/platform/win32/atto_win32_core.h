#pragma once

#include "../../game/atto_core.h"
#include "atto_win32_window.h"

struct GLFWmonitor;
struct GLFWwindow;
struct FONScontext;
typedef FONScontext * FontContext;

namespace atto {
    class OpenglState;
    class VulkanState;
};

namespace atto {
    struct ResourceRegistry {
        FontContext                                 fontContext;
    };

    class WindowsCore : public Core {
    public:
        void Run(int argc, char** argv) override;
        
        virtual f64                     GetTheCurrentTime() const override;

        virtual FontHandle              ResourceGetFont( const char * name ) override;
        virtual float                   FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA, TextAlignment_V vA ) override;

        virtual void                    AudioPlay( AudioResource * audioResource, AudioGroupResource * audioGroup ) override;
        virtual void                    AudioPlayRandomFromGroup( AudioGroupResource * audioGroup ) override;
        virtual void                    AudioSetListener( glm::vec2 pos ) override;

        virtual void                    RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) override;
        
        virtual void                    InputDisableMouse() override;
        virtual void                    InputEnableMouse() override;
        virtual bool                    InputIsMouseDisabled() override;

        virtual void *                  WindowGetHandle() override;
        virtual void                    WindowClose() override;
        virtual void                    WindowSetTitle(const char* title) override;
        virtual void                    WindowSetVSync( bool value ) override;
        virtual bool                    WindowGetVSync() override;
        virtual bool                    WindowIsFullscreen() override;
        virtual bool                    WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) override;
        virtual bool                    WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) override;

        static void                     Win32SetupCrashReporting();

    public:
        ResourceRegistry                    resources = {};

        PlatformWindow                  window;
        VulkanState *                   vkState = nullptr;
        OpenglState *                   glState = nullptr;

        void            OsParseStartArgs( int argc, char ** argv );

        bool            AudioInitialize();
        void            AudioUpdate();
        void            AudioShudown();

        void            VkStart();
        void            GLStart();
    };
}

