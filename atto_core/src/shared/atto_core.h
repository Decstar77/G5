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

struct GGPOSession;

namespace atto {
    class Core;
    class Game;
    class NetClient;

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

    REFL_DECLARE( GameSettings );
    REFL_VAR( GameSettings, windowWidth );
    REFL_VAR( GameSettings, windowHeight );
    REFL_VAR( GameSettings, windowStartPosX );
    REFL_VAR( GameSettings, windowStartPosY );
    REFL_VAR( GameSettings, noAudio );
    REFL_VAR( GameSettings, fullscreen );
    REFL_VAR( GameSettings, vsync );
    REFL_VAR( GameSettings, showDebug );
    REFL_VAR( GameSettings, basePath );

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

    struct AudioSpeaker {
        u32         sourceHandle;
        i32         index;
    };
    enum class RESOURCE_HANDLE_FONT {};
    typedef ObjectHandle<RESOURCE_HANDLE_FONT> FontHandle;

    enum class DrawCommandType {
        NONE = 0,
        CIRCLE,
        RECT,
        SPRITE,
        TEXT,
        LINE,
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
                    TextureResource * textureRes;
                } sprite;
                struct {
                    glm::vec2 bl;
                    glm::mat4 proj;
                    SmallString text;
                    f32 fontSize;
                    FontHandle font;
                } text;
            };
        };
    };

    struct RenderCommands {
        FixedList<DrawCommand, 1024> drawList;
    };

    enum PlayerConnectState {
        PLAYER_CONNECTION_STATE_CONNECTING = 0,
        PLAYER_CONNECTION_STATE_SYNCHRONIZING,
        PLAYER_CONNECTION_STATE_RUNNING,
        PLAYER_CONNECTION_STATE_DISCONNECTED,
        PLAYER_CONNECTION_STATE_DISCONNECTING,
    };

    inline SmallString PlayerConnectStateToString( PlayerConnectState state ) {
        switch( state ) {
            case PLAYER_CONNECTION_STATE_CONNECTING: return SmallString::FromLiteral( "Connecting" );
            case PLAYER_CONNECTION_STATE_SYNCHRONIZING: return SmallString::FromLiteral( "Synchronizing" );
            case PLAYER_CONNECTION_STATE_RUNNING: return SmallString::FromLiteral( "Running" );
            case PLAYER_CONNECTION_STATE_DISCONNECTED: return SmallString::FromLiteral( "Disconnected" );
            case PLAYER_CONNECTION_STATE_DISCONNECTING: return SmallString::FromLiteral( "Disconnecting" );
            default: return SmallString::FromLiteral( "Unknown" );
        }
    }

    struct UIButton {
        glm::vec2 textPos;
        glm::vec2 center;
        glm::vec2 size;
        glm::vec4 color;
        SmallString text;
    };

    struct UIState {
        FixedList<UIButton, 64> buttons;
    };

    class Core {
    public:

        void                                LogOutput( LogLevel level, const char * message, ... );

        f32                                 GetDeltaTime() const;
        f64                                 GetLastTime() const;

        virtual TextureResource *           ResourceGetAndLoadTexture( const char * name ) = 0;
        virtual AudioResource *             ResourceGetAndLoadAudio( const char * name ) = 0;
        virtual FontHandle                  ResourceGetFont( const char * name ) = 0;

        void                                UIBegin();
        bool                                UIPushButton( const char * text, glm::vec2 center, glm::vec4 color = glm::vec4( 1 ) );
        void                                UIEndAndRender();

        glm::vec2                           ViewPosToScreenPos( glm::vec2 worldPos );
        glm::vec2                           ScreenPosToViewPos( glm::vec2 screenPos );

        glm::vec2                           ScreenPosToWorldPos( glm::vec2 screenPos );
        glm::vec2                           WorldPosToScreenPos( glm::vec2 worldPos );

        f32                                 ScreenLengthToWorldLength( f32 screenLength );
        f32                                 WorldLengthToScreenLength( f32 worldLength );

        void                                RenderDrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawLine( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawSprite( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawSpriteBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawText( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, glm::vec4 colour = glm::vec4( 1 ) );
        virtual void                        RenderSetCamera( f32 width, f32 height ) = 0;
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
        glm::vec2                           InputMousePosPixels();
        glm::vec2                           InputMousePosWorld();
        FrameInput & InputGetFrameInput();

        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle( const char * title ) = 0;
        //virtual void                        WindowSetFullscreen(bool fullscreen) = 0;
        //virtual void                        WindowSetCursorVisible(bool visible) = 0;
        //virtual void                        WindowSetCursorLocked(bool locked) = 0;

        NetClient * GetNetClient();

        virtual void                        Run( int argc, char ** argv ) = 0;

    protected:
        GameSettings        theGameSettings = {};
        LoggingState        logger = {};
        RenderCommands      drawCommands = {};
        FrameInput          input = {};
        UIState             uiState = {};

        Game * game = nullptr;

        NetClient * client = nullptr;

        f64                 currentTime = 0.0f;
        f32                 deltaTime = 0.0f;

        f32                 cameraWidth;
        f32                 cameraHeight;
        glm::vec2           cameraPos;
        f32                 mainSurfaceWidth;
        f32                 mainSurfaceHeight;
        glm::vec4           viewport;
        glm::mat4           screenProjection;
        glm::mat4           cameraProjection;

        u8 * thePermanentMemory = nullptr;
        u64 thePermanentMemorySize = 0;
        u64 thePermanentMemoryCurrent = 0;
        std::mutex thePermanentMemoryMutex;

        u8 * theTransientMemory = nullptr;
        u64 theTransientMemorySize = 0;
        u64 theTransientMemoryCurrent = 0;
        std::mutex theTransientMemoryMutex;

        void    MemoryMakePermanent( u64 bytes );
        void    MemoryClearPermanent();
        void    MemoryMakeTransient( u64 bytes );
        void    MemoryClearTransient();

        void    GGPOStartSession( i32 local, i32 peer );
        void    GGPOPoll();

        virtual u64  OsGetFileLastWriteTime( const char * fileName ) = 0;
        virtual void OsLogMessage( const char * message, u8 colour ) = 0;
        virtual void OsErrorBox( const char * msg ) = 0;
    };

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocatePermanent() {
        return (_type_ *)MemoryAllocatePermanent( sizeof( _type_ ) );
    }

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocatePermanentCPP() {
        void * mem = MemoryAllocatePermanent( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocateTransient() {
        return (_type_ *)MemoryAllocateTransient( sizeof( _type_ ) );
    }

    template<typename _type_>
    _type_ * atto::Core::MemoryAllocateTransientCPP() {
        void * mem = MemoryAllocateTransient( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }


}
