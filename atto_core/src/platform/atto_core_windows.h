#pragma once

#include "../shared/atto_core.h"
#include "atto_platform_window.h"

struct GLFWmonitor;
struct GLFWwindow;
struct FONScontext;
typedef FONScontext * FontContext;

namespace FMOD {
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
}

namespace atto {
    class OpenglState;
    class VulkanState;
};

namespace atto {
    struct FMODAudioResource : public AudioResource {
        FMOD::Sound * sound2D;
        FMOD::Sound * sound3D;
    };

    struct ResourceRegistry {
        FixedList<FMODAudioResource, 1024>          audios;
        FixedList<SpriteResource, 1024>             sprites;
        FontContext                                 fontContext;
    };

    class WindowsCore : public Core {
    public:
        void Run(int argc, char** argv) override;
        
        virtual f64                     GetTheCurrentTime() const override;

        virtual TextureResource *       ResourceGetAndCreateTexture( const char * name, bool genMips, bool genAnti ) override;
        virtual TextureResource *       ResourceGetAndLoadTexture( const char * name ) override;
        virtual AudioResource *         ResourceGetAndCreateAudio( const char * name, bool is2D, bool is3D, f32 minDist, f32 maxDist ) override;
        virtual AudioResource *         ResourceGetAndLoadAudio( const char * name ) override;

        virtual SpriteResource *        ResourceGetAndCreateSprite( const char * spriteName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) override;
        virtual SpriteResource *        ResourceGetAndLoadSprite( const char * spriteName ) override;
        virtual SpriteResource *        ResourceGetLoadedSprite( i64 spriteId );

        virtual FontHandle              ResourceGetFont( const char * name ) override;
        virtual float                   FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA, TextAlignment_V vA ) override;

        virtual void                    ResourceReadEntireTextFile( const char * path, char * data, i32 maxLen ) override;
        virtual void                    ResourceWriteEntireTextFile( const char * path, const char * data ) override;
        virtual void                    ResourceReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) override;
        virtual void                    ResourceWriteEntireBinaryFile( const char * path, const char * data, i32 size ) override;
        virtual i64                     ResourceGetFileSize( const char * path ) override;

        virtual void                    AudioPlay( AudioResource * audioResource, glm::vec2 * pos ) override;
        virtual void                    AudioSetListener( glm::vec2 pos ) override;

        virtual void                    RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) override;
        
        virtual void                    InputDisableMouse() override;
        virtual void                    InputEnableMouse() override;
        virtual bool                    InputIsMouseDisabled() override;

        virtual void                    WindowClose() override;
        virtual void                    WindowSetTitle(const char* title) override;
        virtual void                    WindowSetVSync( bool value ) override;
        virtual bool                    WindowGetVSync() override;
        virtual bool                    WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) override;
        virtual bool                    WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) override;

        static void                     Win32SetupCrashReporting();
        static void                     WinBoyoWriteTextFile( const char * path, const char * text );
        static void                     WinBoyoReadTextFile( const char * path, char * text, i32 maxLen );
        static void                     WinBoyoWriteBinaryFile( const char * path, const char * data, i64 size );
        static void                     WinBoyoReadBinaryFile( const char * path, char * data, i64 size );

    #if ATTO_EDITOR
        virtual void                    EditorOnly_SaveLoadedResourcesToBinary() override;
    #endif

    public:
        ResourceRegistry                    resources = {};

        FMOD::ChannelGroup *                fmodMasterGroup;
        FMOD::System *                      fmodSystem;
        FixedObjectPool<AudioSpeaker, 1024> fmodSpeakers;
        FixedList<AudioSpeaker * , 1024>    fmodActives;

        PlatformWindow                  window;
        VulkanState *                   vkState = nullptr;
        OpenglState *                   glState = nullptr;

        u64             OsGetFileLastWriteTime( const char * fileName ) override;
        void            OsLogMessage( const char * message, u8 colour ) override;
        void            OsErrorBox( const char * msg ) override;
        void            OsParseStartArgs( int argc, char ** argv );

        bool            AudioInitialize();
        void            AudioUpdate();
        void            AudioShudown();

        void            VkStart();
        void            GLStart();
    };
}

