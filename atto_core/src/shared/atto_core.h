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
                    FontResource * fontRes;
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
            case PLAYER_CONNECTION_STATE_CONNECTING: return SmallString::FromLiteral("Connecting");
            case PLAYER_CONNECTION_STATE_SYNCHRONIZING: return SmallString::FromLiteral("Synchronizing");
            case PLAYER_CONNECTION_STATE_RUNNING: return SmallString::FromLiteral("Running");
            case PLAYER_CONNECTION_STATE_DISCONNECTED: return SmallString::FromLiteral("Disconnected");
            case PLAYER_CONNECTION_STATE_DISCONNECTING: return SmallString::FromLiteral("Disconnecting");
            default: return SmallString::FromLiteral("Unknown");
        }
    }

    struct PlayerConnectionInfo {
        PlayerConnectState  state;
        i32                 playerHandle;
        i32                 playerNumber;
        i32                 connectProgress;
        i32                 disconnectTimeout;
        i32                 disconnectStart;
        i32                 pingToPeer;
        i32                 kbsSent;
    };
    
    enum MapTileType {
        MAP_TILE_TYPE_NONE = 0,
        MAP_TILE_TYPE_METAL_WALL,
    };

    struct MapElement {
        MapTileType type;
        i32 index;
        fpv2 pos;
        fp rot;
    };

    enum SimInput {
        SIM_INPUT_NONE                      = 0,
        SIM_INPUT_TANK_LEFT                 = SetABit( 1 ),
        SIM_INPUT_TANK_RIGHT                = SetABit( 2 ),
        SIM_INPUT_TANK_UP                   = SetABit( 3 ),
        SIM_INPUT_TANK_DOWN                 = SetABit( 4 ),
        SIM_INPUT_TANK_TURRET_LEFT          = SetABit( 5 ),
        SIM_INPUT_TANK_TURRET_RIGHT         = SetABit( 6 ),
    };

    struct SimTank {
        fpv2    pos;
        fp      rot;
        fp      turretRot;
        i32     health;
    };

    struct SimState {
        FixedList<MapElement, 2048> elements;
        FixedList<SimTank, 2>       playerTanks;
    };

    enum SimGameType {
        SIM_GAME_TYPE_NONE = 0,
        SIM_GAME_TYPE_SINGLE_PLAYER,
        SIM_GAME_TYPE_MULTI_PLAYER,
    };

    class SimLogic {
    public:
        void StartSinglePlayerGame();
        void StartMultiplayerGame();

        void Start();
        void Advance( i32 playerOneInput, i32 playerTwoInput, i32 dcFlags);
        void LoadState( u8 * buffer, i32 len );
        void SaveState( u8 ** buffer, i32 * len, i32 * checksum, i32 frame );
        void FreeState( void * buffer );
        void LogState( char * filename, u8 * buffer, i32 len );

        void SkipNextUpdates( i32 count );
        i32 GetNextInputs( i32 localPLayerNumber );

        i32 mapWidth = 1280;
        i32 mapHeight = 720;

        i32 skipNextSteps = 0;
        SimState state = {};
        Core * core = nullptr;
        SimGameType gameType = SimGameType::SIM_GAME_TYPE_NONE;
        bool isRunning = false;
    };

    enum class GameLocationState {
        NONE = 0,
        MAIN_MENU,
        IN_GAME,
        OPTIONS,
    };

    class GameLogic {
    public:
        void Start( Core * core );
        void UpdateAndRender( Core * core, SimLogic * sim );
        void Shutdown( Core * core );

        void MainMenuUpdateAndRender( Core * core, SimLogic * sim );
        void InGameUpdateAndRender( Core * core, SimLogic * sim );
        void OptionsUpdateAndRender( Core * core, SimLogic * sim );

        FontResource *      arialFont;
        GameLocationState   currentState = GameLocationState::NONE;
        glm::vec2           p1VisPos;
        f32                 p1VisTurretRot;
        glm::vec2           p2VisPos;
    };

#define MP_FRAME_DELAY 2
#define MP_MAX_INPUTS 2
#define MP_PLAYER_COUNT 2
    struct MultiplayerState {
        GGPOSession *           session;
        union {
            PlayerConnectionInfo players[ MP_PLAYER_COUNT ];
            struct {
                PlayerConnectionInfo    local;
                PlayerConnectionInfo    peer;
            };
        };
        
        FixedQueue<NetworkMessage, 1024> messages;

        void SetConnectionState( PlayerConnectState state, i32 arg1, i32 arg2 );
        void SetConnectionState( int playerNumber, PlayerConnectState state, i32 arg1, i32 arg2 );
        void GatherNetworkStats();
    };

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
        virtual FontResource *              ResourceGetAndLoadFont( const char * name, i32 fontSize ) = 0;

        f32                                 FontGetWidth( FontResource * font, const char * text );
        f32                                 FontGetHeight( FontResource * font );

        void                                UIBegin();
        bool                                UIPushButton( const char * text, glm::vec2 center, glm::vec4 color = glm::vec4( 1 ) );
        void                                UIEndAndRender();

        glm::vec2                           WorldPosToScreenPos( glm::vec2 worldPos );
        glm::vec2                           ScreenPosToWorldPos( glm::vec2 screenPos );

        void                                RenderDrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawLine( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void                                RenderDrawSprite( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawSpriteBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void                                RenderDrawText( FontResource * font, glm::vec2 bl, const char * text, glm::vec4 colour = glm::vec4( 1 ) );
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
        FrameInput &                        InputGetFrameInput();

        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle( const char * title ) = 0;
        //virtual void                        WindowSetFullscreen(bool fullscreen) = 0;
        //virtual void                        WindowSetCursorVisible(bool visible) = 0;
        //virtual void                        WindowSetCursorLocked(bool locked) = 0;

        NetClient *                         GetNetClient();
        SimLogic *                          GetSimLogic();
        MultiplayerState *                  GetMPState();

        FixedQueue<NetworkMessage, 1024> &  GetGGPOMessages();

        FontResource *                      GetDebugFont();

        virtual void                        Run( int argc, char ** argv ) = 0;

    protected:
        GameSettings        theGameSettings = {};
        LoggingState        logger = {};
        RenderCommands      drawCommands = {};
        FrameInput          input = {};
        MultiplayerState    mpState = {};
        UIState             uiState = {};
        

        NetClient * client = nullptr;
        SimLogic * simLogic = nullptr;
        GameLogic * gameLogic = nullptr;

        FontResource *      arialFont = NULL;

        f64                 currentTime = 0.0f;
        f32                 deltaTime = 0.0f;

        f32                 cameraWidth;
        f32                 cameraHeight;
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
