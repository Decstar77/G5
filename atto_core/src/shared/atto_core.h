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
    class GameMode;
    class Editor;
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

        REFLECT();
    };

    struct TextureResource {
        SmallString name;
        i32 width;
        i32 height;
        i32 channels;
        bool hasMips;
        bool hasAnti;
    };

    struct AudioResource {
        SmallString name;
        i32 channels;
        i32 sampleRate;
        i32 sizeBytes;
        i32 bitDepth;
    };

    struct StaticMeshResource {
        SmallString name;
        i32 vertexCount;
        i32 indexCount;
        i32 vertexStride;
        i32 indexStride;
        bool updateable;
    };

    struct StaticMeshVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct StaticMeshData {
        SmallString         name;
        i32                 vertexCount;
        StaticMeshVertex *  vertices;
        i32                 indexCount;
        u16 *               indices;

        void Free();
    };

    struct StaticModelResource {
        SmallString                             name;
        FixedList< StaticMeshResource *, 16>    meshes;
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
        LINE2D,
        SPRITE,
        TEXT,

        // 3D
        PLANE,
        SPHERE,
        LINE,
        TRIANGLE,
        MESH,
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
                    glm::vec2 p1;
                    glm::vec2 p2;
                    glm::vec2 p3;
                    glm::vec2 p4;
                    glm::mat4 p;
                } line2D;
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
                struct {
                    glm::vec3 center;
                    glm::vec3 normal;
                    glm::vec2 dim;
                } plane;
                struct {
                    glm::vec3 center;
                    f32 r;
                } sphere;
                struct {
                    glm::vec3 p1;
                    glm::vec3 p2;
                } line;
                struct {
                    glm::vec3 p1;
                    glm::vec3 p2;
                    glm::vec3 p3;
                    glm::vec2 uv1;
                    glm::vec2 uv2;
                    glm::vec2 uv3;
                    TextureResource * texture;
                } triangle;
                struct {
                    glm::mat4 m;
                    StaticMeshResource * mesh;
                    TextureResource * albedo;
                } mesh;
            };
        };
    };

    struct Camera {
        glm::vec3 pos;
        glm::vec3 dir;
        f32 yfov;
        f32 zNear;
        f32 zFar;
    };

    class DrawContext {
        friend class Core;
        friend class WindowsCore;
    public:
        void SetCamera( glm::mat4 view, f32 yfov, f32 zNear, f32 zFar );

        void DrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine2D( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine2D_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawSprite( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSpriteBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawText2D( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawPlane( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSphere( glm::vec3 center, f32 r, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture );

        void DrawMesh( StaticMeshResource * mesh, glm::mat4 m, TextureResource * albedo = nullptr );

        inline glm::vec2 GetMainSurfaceDims() const { return glm::vec2( mainSurfaceWidth, mainSurfaceHeight ); }
        inline f32 GetMainAspectRatio() const { return mainAspect; }

    private:
        glm::mat4       cameraProj;
        glm::mat4       cameraView;

        f32             mainSurfaceWidth;
        f32             mainSurfaceHeight;
        f32             mainAspect;
        glm::mat4       screenProjection;
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
        Camera                              CreateDefaultCamera() const;
        
        virtual TextureResource *           ResourceGetAndLoadTexture( const char * name, bool genMips, bool genAnti ) = 0;
        virtual AudioResource *             ResourceGetAndLoadAudio( const char * name ) = 0;
        virtual StaticMeshResource *        ResourceGetAndLoadMesh( const char * name ) = 0;
        virtual FontHandle                  ResourceGetFont( const char * name ) = 0;
        virtual void                        ResourceReadEntireFile( const char * path, char * data, i32 maxLen ) = 0;
        virtual void                        ResourceWriteEntireFile( const char * path, const char * data ) = 0;

        DrawContext *                       RenderGetDrawContext( i32 index, bool clear = true );
        virtual void                        RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) = 0;

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
        bool                                InputMouseHasMoved();
        glm::vec2                           InputMousePosNDC();
        glm::vec2                           InputMousePosPixels();
        glm::vec2                           InputMouseDeltaPixels();
        FrameInput &                        InputGetFrameInput();
        virtual void                        InputDisableMouse() = 0;
        virtual void                        InputEnableMouse() = 0;
        virtual bool                        InputIsMouseDisabled() = 0;

        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle( const char * title ) = 0;
        virtual void                        WindowSetVSync( bool value ) = 0;
        virtual bool                        WindowGetVSync() = 0;
        //virtual void                        WindowSetFullscreen(bool fullscreen) = 0;
        //virtual void                        WindowSetCursorVisible(bool visible) = 0;
        //virtual void                        WindowSetCursorLocked(bool locked) = 0;

        NetClient *                         GetNetClient();

        virtual void                        Run( int argc, char ** argv ) = 0;

    protected:
        GameSettings                theGameSettings = {};
        LoggingState                logger = {};
        FixedList<DrawContext, 8>   drawContexts = {};
        FrameInput                  input = {};
        UIState                     uiState = {};

        GameMode *                  game = nullptr;
        Editor *                    editor = nullptr;

        NetClient * client = nullptr;

        f64                 currentTime = 0.0f;
        f32                 deltaTime = 0.0f;

        f32                 mainSurfaceWidth;
        f32                 mainSurfaceHeight;
        glm::mat4           screenProjection;

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
