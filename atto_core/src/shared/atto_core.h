#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"
#include "atto_logging.h"
#include "atto_input.h"

#include "atto_reflection.h"
#include <memory>
#include <mutex>

namespace atto {

    class Core;
    struct GameCodeAPI {
        typedef u64  (*GameSize)();
        typedef void (*GameStart)(Core* core);
        typedef void (*GameUpdateAndRender)(Core* core);
        typedef void (*GameShutdown)(Core* core);
        typedef void (*GameSimStart)(Core* core);
        typedef void (*GameSimStep)(Core* core, const FixedList<i32, 2>& inputs);
        typedef void (*GameSimSave)(Core* core, void** buffer, i64& size, i64 &checkSum);
        typedef void (*GameSimLoad)(Core* core, void* buffer, i64 size);
        GameSize            gameSize;
        GameStart           gameStart;
        GameUpdateAndRender gameUpdateAndRender;
        GameShutdown        gameShutdown;
        GameSimStart        gameSimStart;
        GameSimStep         gameSimStep;
        GameSimSave         gameSimSave;
        GameSimLoad         gameSimLoad;
    };

    ATTO_REFLECT_STRUCT(GameSettings)
    struct GameSettings {
        i32             windowWidth;
        i32             windowHeight;
        i32             windowStartPosX;
        i32             windowStartPosY;
        bool            noAudio;
        bool            fullscreen;
        bool            vsync;
        bool            showDebug;
        SmallString     basePath;
    };

    struct TextureResource {
        SmallString name;
        i32 width;
        i32 height;
        i32 channels;
        bool generateMipMaps;
    };
    
    struct AudioResource {
        SmallString name;
        i32 channels;
        i32 sampleRate;
        i32 sizeBytes;
        i32 bitDepth;
    };

    struct FontChar {
        u32 textureId; //@TODO: BAD!!!
        glm::vec2 size;
        glm::vec2 bearing;
        u32 advance;
    };

    struct FontResource {
        SmallString name;
        i32 fontSize;
        FixedList<FontChar, 128> chars;
    };

    struct AudioSpeaker {
        u32         sourceHandle;
        i32         index;
    };

    class Camera {
    public:
        glm::vec2 pos;
        f32 zoom;
        i32 width;
        i32 height;
        i32 mainSurfaceWidth;
        i32 mainSurfaceHeight;
        glm::mat4 p;
        glm::mat4 v;

        glm::vec2 ScreenPointToWorld(glm::vec2 screen);
        glm::vec2 WorldPointToScreen(glm::vec2 world);
    };

    enum class DrawCommandType {
        NONE = 0,
        CIRCLE,
        RECT,
        SPRITE,
        TEXT,
    };

    struct DrawCommand {
        DrawCommandType type;
        glm::vec4 color;
        struct {
            union {
                struct {
                    glm::vec2 c;
                    f32 r;
                } circle;
                struct {
                    glm::vec2 tr;
                    glm::vec2 br;
                    glm::vec2 bl;
                    glm::vec2 tl;
                } rect;
                struct {
                    glm::vec2 tr;
                    glm::vec2 br;
                    glm::vec2 bl;
                    glm::vec2 tl;
                    glm::mat4 proj;
                    TextureResource* textureRes;
                } sprite;
                struct {
                    glm::vec2 bl;
                    glm::mat4 proj;
                    SmallString text;
                    FontResource* fontRes;
                } text;
            };
        };
    };

    struct RenderCommands {
        FixedList<DrawCommand, 1024> drawList;
    };

    class Core {
    public:
        
        void                                LogOutput(LogLevel level, const char* message, ...);

        virtual TextureResource*            ResourceGetAndLoadTexture(const char* name) = 0;
        virtual AudioResource*              ResourceGetAndLoadAudio(const char* name) = 0;
        virtual FontResource*               ResourceGetAndLoadFont(const char* name, i32 fontSize) = 0;

        Camera                              RenderCreateCamera();
        void                                RenderSetCamera(Camera* camera);
        void                                RenderDrawCircle(glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4(1));
        void                                RenderDrawRect(glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4(1));
        void                                RenderDrawRect(glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4& color = glm::vec4(1));
        void                                RenderDrawLine(glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4& color = glm::vec4(1));
        void                                RenderDrawSprite(TextureResource* texture, glm::vec2 center, glm::vec2 size = glm::vec2(1), glm::vec4 colour = glm::vec4(1));
        void                                RenderDrawText(FontResource* font, glm::vec2 bl, const char* text, glm::vec4 colour = glm::vec4(1));
        virtual void                        RenderSubmit() = 0;

        virtual AudioSpeaker                AudioPlay(AudioResource* audioResource, f32 volume = 1.0f, bool looping = false) = 0;

        void                                NetConnect();
        bool                                NetIsConnected();
        void                                NetDisconnect();
        SmallString                         NetStatusText();
        u32                                 NetGetPing();

        void*                               MemoryAllocatePermanent(u64 bytes);
        void*                               MemoryAllocateTransient(u64 bytes);
        template<typename _type_> _type_*   MemoryAllocateTransient();
        template<typename _type_> _type_*   MemoryAllocateTransientCPP();
        template<typename _type_> _type_*   MemoryAllocatePermanent();
        template<typename _type_> _type_*   MemoryAllocatePermanentCPP();

        bool                                InputKeyDown(KeyCode keyCode);
        bool                                InputKeyUp(KeyCode keyCode);
        bool                                InputKeyJustPressed(KeyCode keyCode);
        bool                                InputKeyJustReleased(KeyCode keyCode);
        bool                                InputMouseButtonDown(MouseButton button);
        bool                                InputMouseButtonUp(MouseButton button);
        bool                                InputMouseButtonJustPressed(MouseButton button);
        bool                                InputMouseButtonJustReleased(MouseButton button);
        FrameInput&                         InputGetFrameInput();

        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle(const char* title) = 0;
        //virtual void                        WindowSetFullscreen(bool fullscreen) = 0;
        //virtual void                        WindowSetCursorVisible(bool visible) = 0;
        //virtual void                        WindowSetCursorLocked(bool locked) = 0;

        virtual void                        Run(int argc, char** argv) = 0;

        i32 localPlayerNumber;
        glm::vec2 p1Pos;
        glm::vec2 p2Pos;

    public:
        void* UserData = nullptr; // for the game to store whatever it wants

    protected:
        GameSettings        theGameSettings = {};
        LoggingState        logger = {};
        RenderCommands      drawCommands = {};
        FrameInput          input = {};

        i32                 mainSurfaceWidth;
        i32                 mainSurfaceHeight;
        glm::mat4           screenProjection;

        Camera *            camera = {};

        u8* thePermanentMemory = nullptr;
        u64 thePermanentMemorySize = 0;
        u64 thePermanentMemoryCurrent = 0;
        std::mutex thePermanentMemoryMutex;

        u8* theTransientMemory = nullptr;
        u64 theTransientMemorySize = 0;
        u64 theTransientMemoryCurrent = 0;
        std::mutex theTransientMemoryMutex;

        class NetClient* client = nullptr;

        void    MemoryMakePermanent(u64 bytes);
        void    MemoryClearPermanent();
        void    MemoryMakeTransient(u64 bytes);
        void    MemoryClearTransient();

        virtual u64  OsGetFileLastWriteTime(const char* fileName) = 0;
        virtual bool OsLoadDLL(GameCodeAPI & gameCode) = 0;
        virtual void OsLogMessage(const char* message, u8 colour) = 0;
        virtual void OsErrorBox(const char* msg) = 0;
    };

    template<typename _type_>
    _type_* atto::Core::MemoryAllocatePermanent()
    {
        return (_type_*)MemoryAllocatePermanent(sizeof(_type_));
    }

    template<typename _type_>
    _type_* atto::Core::MemoryAllocatePermanentCPP()
    {
        void* mem = MemoryAllocatePermanent(sizeof(_type_));
        return new (mem) _type_;
    }

    template<typename _type_>
    _type_* atto::Core::MemoryAllocateTransient()
    {
        return (_type_*)MemoryAllocateTransient(sizeof(_type_));
    }


    template<typename _type_>
    _type_* atto::Core::MemoryAllocateTransientCPP()
    {
        void* mem = MemoryAllocateTransient(sizeof(_type_));
        return new (mem) _type_;
    }


}
