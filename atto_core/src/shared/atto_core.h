#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"
#include "atto_logging.h"
#include "atto_input.h"
#include "atto_network.h"

#include "atto_reflection.h"

#include <memory>
#include <mutex>

namespace atto {
    class Core;
    class NetClient;

    ATTO_REFLECT_STRUCT( GameSettings )
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

        glm::vec2 ScreenPointToWorld( glm::vec2 screen );
        glm::vec2 WorldPointToScreen( glm::vec2 world );
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
                    TextureResource * textureRes;
                } sprite;
                struct {
                    glm::vec2 bl;
                    glm::mat4 proj;
                    SmallString text;
                    FontResource * fontRes;
                } text;
            };
        };
    };

    struct RenderCommands {
        FixedList<DrawCommand, 1024> drawList;
    };

    typedef int PlayerHandle; // For GGPO

    enum PlayerConnectState {
        PLAYER_CONNECTION_STATE_CONNECTING = 0,
        PLAYER_CONNECTION_STATE_SYNCHRONIZING,
        PLAYER_CONNECTION_STATE_RUNNING,
        PLAYER_CONNECTION_STATE_DISCONNECTED,
        PLAYER_CONNECTION_STATE_DISCONNECTING,
    };

    class SimLogic {
    public:
        void Advance();
        void LoadState( u8 * buffer, i32 len );
        void SaveState( u8 ** buffer, i32 * len, i32 * checksum, i32 frame );
        void FreeState( void * buffer );
        void LogState( char * filename, u8 * buffer, i32 len );

        void SetConnectionState( PlayerConnectState state, i32 arg1, i32 arg2 );
        void SetConnectionState( PlayerHandle player, PlayerConnectState state, i32 arg1, i32 arg2 );
        void SkipNextUpdates( int count );

        //virtual void ApplyAction(Core *core, SimAction action);

        glm::vec2 p1Pos;
        glm::vec2 p2Pos;
        i32 inputForNextSim = 0;
    };

    class GameLogic {
    public:
        void Start( Core * core );
        void UpdateAndRender( Core * core, SimLogic * sim );
        void Shutdown( Core * core );

        SmallString     debugText;
        FontResource * arialFont;

        glm::vec2 p1VisPos;
        glm::vec2 p2VisPos;
    };

    class Core {
    public:

        void                                LogOutput( LogLevel level, const char * message, ... );

        f32                                 GetDeltaTime() const;
        f64                                 GetLastTime() const;

        virtual TextureResource *           ResourceGetAndLoadTexture( const char * name ) = 0;
        virtual AudioResource *             ResourceGetAndLoadAudio( const char * name ) = 0;
        virtual FontResource *              ResourceGetAndLoadFont( const char * name, i32 fontSize ) = 0;

        Camera                              RenderCreateCamera();
        void                                RenderSetCamera( Camera * camera );
        void                                RenderDrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawLine( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawSprite( TextureResource * texture, glm::vec2 center, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawText( FontResource * font, glm::vec2 bl, const char * text, glm::vec4 colour = glm::vec4( 1 ) );
        virtual void                        RenderSubmit() = 0;

        virtual AudioSpeaker                AudioPlay( AudioResource * audioResource, f32 volume = 1.0f, bool looping = false ) = 0;

        void                                NetConnect();
        bool                                NetIsConnected();
        void                                NetDisconnect();
        SmallString                         NetStatusText();
        u32                                 NetGetPing();

        void *                              MemoryAllocatePermanent( u64 bytes );
        void *                              MemoryAllocateTransient( u64 bytes );
        template<typename _type_> _type_ *  MemoryAllocateTransient();
        template<typename _type_> _type_ *  MemoryAllocateTransientCPP();
        template<typename _type_> _type_ *  MemoryAllocatePermanent();
        template<typename _type_> _type_ *  MemoryAllocatePermanentCPP();

        bool                                InputKeyDown( KeyCode keyCode );
        bool                                InputKeyUp( KeyCode keyCode );
        bool                                InputKeyJustPressed( KeyCode keyCode );
        bool                                InputKeyJustReleased( KeyCode keyCode );
        bool                                InputMouseButtonDown( MouseButton button );
        bool                                InputMouseButtonUp( MouseButton button );
        bool                                InputMouseButtonJustPressed( MouseButton button );
        bool                                InputMouseButtonJustReleased( MouseButton button );
        FrameInput & InputGetFrameInput();

        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle( const char * title ) = 0;
        //virtual void                        WindowSetFullscreen(bool fullscreen) = 0;
        //virtual void                        WindowSetCursorVisible(bool visible) = 0;
        //virtual void                        WindowSetCursorLocked(bool locked) = 0;

        NetClient *                         GetNetClient();
        SimLogic *                          GetSimLogic();

        FixedQueue<NetworkMessage, 1024> &  GetGGPOMessages();

        virtual void                        Run( int argc, char ** argv ) = 0;

    protected:
        GameSettings        theGameSettings = {};
        LoggingState        logger = {};
        RenderCommands      drawCommands = {};
        FrameInput          input = {};

        NetClient * client = nullptr;
        SimLogic * simLogic = nullptr;
        GameLogic * gameLogic = nullptr;

        f64                 currentTime = 0.0f;
        f32                 deltaTime = 0.0f;

        i32                 mainSurfaceWidth;
        i32                 mainSurfaceHeight;
        glm::mat4           screenProjection;

        Camera * camera = {};

        u8 * thePermanentMemory = nullptr;
        u64 thePermanentMemorySize = 0;
        u64 thePermanentMemoryCurrent = 0;
        std::mutex thePermanentMemoryMutex;

        u8 * theTransientMemory = nullptr;
        u64 theTransientMemorySize = 0;
        u64 theTransientMemoryCurrent = 0;
        std::mutex theTransientMemoryMutex;

        FixedQueue<NetworkMessage, 1024> ggpoMessages = {};

        void    MemoryMakePermanent( u64 bytes );
        void    MemoryClearPermanent();
        void    MemoryMakeTransient( u64 bytes );
        void    MemoryClearTransient();

        void    GGPOStartSession( PlayerHandle local, PlayerHandle peer );

        virtual u64  OsGetFileLastWriteTime( const char * fileName ) = 0;
        virtual void OsLogMessage( const char * message, u8 colour ) = 0;
        virtual void OsErrorBox( const char * msg ) = 0;
    };

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocatePermanent()
    {
        return (_type_ *)MemoryAllocatePermanent( sizeof( _type_ ) );
    }

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocatePermanentCPP()
    {
        void * mem = MemoryAllocatePermanent( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocateTransient()
    {
        return (_type_ *)MemoryAllocateTransient( sizeof( _type_ ) );
    }


    template<typename _type_>
    _type_ * atto::Core::MemoryAllocateTransientCPP()
    {
        void * mem = MemoryAllocateTransient( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }


}
