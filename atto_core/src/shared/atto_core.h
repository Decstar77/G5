#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"
#include "atto_logging.h"
#include "atto_input.h"
#include "atto_network.h"
#include "atto_clock.h"

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
        f32             masterVolume;
        
        static GameSettings CreateSensibleDefaults();

        REFLECT();
    };

    struct TextureResource {
        LargeString name;
        i32 width;
        i32 height;
        i32 channels;
        bool hasMips;
        bool hasAnti;
        u32 handle;
    };

    struct AudioResource {
        LargeString name;
    };

    struct SpriteActuation {
        i32                              frameIndex;
        FixedList< SmallString,     4 >  audioIds;
        FixedList< AudioResource *, 4 >  audioResources;

        REFLECT();
    };

    class SpriteResource {
    public:
        i64                             spriteId; // Deduced from the name and texture name
        LargeString                     spriteName;
        TextureResource *               textureResource;
        bool                            isTileMap;
        i32                             tileWidth;
        i32                             tileHeight;
        i32                             frameCount;
        i32                             frameWidth;
        i32                             frameHeight;
        i32                             frameRate;
        glm::vec2                       origin;
        FixedList< SpriteActuation, 4 > frameActuations;

        LargeString                     GetResourcePath() const;

        REFLECT();
    };

    struct StaticMeshResource {
        SmallString name;
        BoxBounds boundingBox;
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
        i32             index;
    };

    enum class RESOURCE_HANDLE_FONT {};
    typedef ObjectHandle<RESOURCE_HANDLE_FONT> FontHandle;

    enum class DrawCommandType {
        NONE = 0,
        CIRCLE,
        RECT,
        LINE2D,
        TEXTURE,
        SPRITE,
        TEXT,

        // 3D
        PLANE,
        SPHERE,
        BOX,
        LINE,
        TRIANGLE,
        MESH,
    };

    struct DrawCommand {
        DrawCommandType type;
        glm::vec4 color;
        glm::mat4 proj;
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
                    TextureResource *   textureRes;
                } texture;
                struct {
                    glm::vec2 tr;
                    glm::vec2 br;
                    glm::vec2 bl;
                    glm::vec2 tl;
                    glm::vec2 blUV;
                    glm::vec2 trUV;
                    glm::vec2 tlUV;
                    glm::vec2 brUV;
                    SpriteResource * spriteRes;
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
                    glm::mat4 m;
                } box;
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
        void SetCameraPos( glm::vec2 pos );
        void DrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRectNoCamOffset( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRectScreen( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine2D( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine2D_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawTexture( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureScreen( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSprite( SpriteResource * sprite, i32 frameIndex, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSprite( SpriteResource * sprite, i32 tileX, i32 tileY, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawText2D( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawPlane( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSphere( glm::vec3 center, f32 r, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawBox( glm::vec3 min, glm::vec3 max, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture );

        void DrawMesh( StaticMeshResource * mesh, glm::mat4 m, TextureResource * albedo = nullptr );

        glm::vec2 ScreenPosToWorldPos( glm::vec2 screenPos );

        inline glm::vec2    GetMainSurfaceDims() const { return glm::vec2( mainSurfaceWidth, mainSurfaceHeight ); }
        inline f32          GetMainAspectRatio() const { return mainAspect; }

        inline f32          GetCameraWidth() const { return cameraWidth; }
        inline f32          GetCameraHeight() const { return cameraHeight; }
        inline glm::vec2    GetCameraDims() const { return glm::vec2( cameraWidth, cameraHeight ); }

    private:
        glm::mat4       cameraProj;
        glm::mat4       cameraView; // For 3D stuffies
        glm::vec2       cameraPos;

        f32             mainSurfaceWidth;
        f32             mainSurfaceHeight;
        f32             mainAspect;
        glm::vec4       viewport;
        f32             cameraWidth;
        f32             cameraHeight;
        glm::mat4       screenProjection;
        FixedList<DrawCommand, 4000> drawList;
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
        virtual f64                         GetTheCurrentTime() const = 0;
        Camera                              CreateDefaultCamera() const;
        
        void                                MoveToGameMode( GameMode * gameMode );
        
        virtual TextureResource *           ResourceGetAndLoadTexture( const char * name, bool genMips, bool genAnti ) = 0;
        virtual SpriteResource *            ResourceGetAndCreateSprite( const char * spriteName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) = 0;
        virtual SpriteResource *            ResourceGetAndLoadSprite( const char * spriteName ) = 0;
        virtual SpriteResource *            ResourceGetLoadedSprite( i64 spriteId ) = 0; 
        virtual AudioResource *             ResourceGetAndLoadAudio( const char * name ) = 0;
        virtual FontHandle                  ResourceGetFont( const char * name ) = 0;
        virtual void                        ResourceReadEntireFile( const char * path, char * data, i32 maxLen ) = 0;
        virtual void                        ResourceWriteEntireFile( const char * path, const char * data ) = 0;
        virtual i64                         ResourceGetFileSize( const char * path ) = 0;
        char *                              ResourceReadEntireFileIntoTransientMemory( const char * path, i64 * size );

        template< typename _type_ >
        void                                ResourceSaveRefl( const _type_ * obj, const char * path );
        template< typename _type_>
        bool                                ResourceLoadRefl( _type_ * obj, const char * path );

        virtual float                       FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds ) = 0;

        DrawContext *                       RenderGetDrawContext( i32 index, bool clear = true );
        f32                                 RenderGetMainSurfaceWidth() const { return mainSurfaceWidth; }
        f32                                 RenderGetMainSurfaceHeight() const { return mainSurfaceHeight; }
        virtual void                        RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) = 0;

        virtual AudioSpeaker                AudioPlay( AudioResource * audioResource, f32 volume = 1.0f, bool looping = false ) = 0;
        virtual AudioSpeaker                AudioPlay( AudioResource * audioResource, glm::vec2 pos, glm::vec2 vel, f32 volume = 1.0f, bool looping = false ) = 0;
        virtual void                        AudioSetListener( glm::vec2 pos, glm::vec2 vel ) = 0;
        template<i32 capcity>
        AudioResource *                     AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, f32 volume = 1.0f, bool looping = false );
        template<i32 capcity>
        AudioResource *                     AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, glm::vec2 pos, glm::vec2 vel, f32 volume = 1.0f, bool looping = false );
        template<typename... args>
        AudioResource *                     AudioPlayRandom( f32 volume, bool looping, args... audioResources );
        template<typename... args>
        AudioResource *                     AudioPlayRandom( f32 volume, bool looping, glm::vec2 pos, glm::vec2 vel, args... audioResources );

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
        virtual bool                        WindowOpenNativeFileDialog( const char * basePath, LargeString & res ) = 0;
        virtual bool                        WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) = 0;

        NetClient *                         GetNetClient();
        void                                NetworkConnect();
        bool                                NetworkIsConnected();
        void                                NetworkDisconnect();
        SmallString                         NetworkGetStatusText();
        void                                NetworkSend( const NetworkMessage & msg );
        bool                                NetworkRecieve( NetworkMessage & msg );
        u32                                 NetworkGetPing();

        virtual void                        Run( int argc, char ** argv ) = 0;

    protected:
        GameSettings                theGameSettings = {};
        LoggingState                logger = {};
        FixedList<DrawContext, 8>   drawContexts = {};
        FrameInput                  input = {};
        UIState                     uiState = {};

        GameMode *                  currentGameMode = nullptr;
        GameMode *                  nextGameMode = nullptr;
        Editor *                    editor = nullptr;
        NetClient *                 client = nullptr;

        f32                 deltaTime = 0.0f;

        f32                 mainSurfaceWidth;
        f32                 mainSurfaceHeight;
        glm::mat4           screenProjection;
        f32                 cameraWidth;
        f32                 cameraHeight;
        glm::mat4           cameraProjection;
        glm::vec4           viewport;

        glm::vec2           listenerPos;
        glm::vec2           listenerVel;

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
    _type_ * Core::MemoryAllocatePermanent() {
        return (_type_ *)MemoryAllocatePermanent( sizeof( _type_ ) );
    }

    template<typename _type_>
    _type_ * Core::MemoryAllocatePermanentCPP() {
        void * mem = MemoryAllocatePermanent( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }

    template<typename _type_>
    _type_ * Core::MemoryAllocateTransient() {
        return (_type_ *)MemoryAllocateTransient( sizeof( _type_ ) );
    }

    template<typename _type_>
    _type_ * Core::MemoryAllocateTransientCPP() {
        void * mem = MemoryAllocateTransient( sizeof( _type_ ) );
        return new ( mem ) _type_;
    }

    template< typename _type_ >
    void Core::ResourceSaveRefl( const _type_ * obj, const char * path ) {
        TypeDescriptor * settingsType = TypeResolver<_type_>::get();
        nlohmann::json j = settingsType->JSON_Write( obj );
        ResourceWriteEntireFile( path, j.dump().c_str() );
    }

    template< typename _type_>
    bool Core::ResourceLoadRefl( _type_ * obj, const char * path ) {
        i64 fileSize;
        char * data = ResourceReadEntireFileIntoTransientMemory( path, &fileSize );
        if( data != nullptr ) {
            nlohmann::json j = nlohmann::json::parse( data );
            TypeDescriptor * settingsType = TypeResolver<_type_>::get();
            settingsType->JSON_Read( j, obj );
            return true;
        }

        return false;
    }

    template<typename... args>
    AudioResource * Core::AudioPlayRandom( f32 volume, bool looping, args... audioResources ) {
        AudioResource * audioResourceArray[] = { audioResources... };
        i32 index = Random::Int( sizeof...( audioResources ) );
        AudioPlay( audioResourceArray[ index ], volume, looping );
        return audioResourceArray[ index ];
    }

    template<typename... args>
    AudioResource * Core::AudioPlayRandom( f32 volume, bool looping, glm::vec2 pos, glm::vec2 vel, args... audioResources ) {
        AudioResource * audioResourceArray[] = { audioResources... };
        i32 index = Random::Int( sizeof...( audioResources ) );
        AudioPlay( audioResourceArray[ index ], pos, vel, volume, looping );
        return audioResourceArray[ index ];
    }

    template<i32 capcity>
    AudioResource * Core::AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, f32 volume /*= 1.0f*/, bool looping /*= false */ ) {
        i32 index = Random::Int( audioResources.GetCount() );
        AudioPlay( audioResources[ index ], volume, looping );
        return audioResources[ index ];
    }

    template<i32 capcity>
    AudioResource * Core::AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, glm::vec2 pos, glm::vec2 vel, f32 volume /*= 1.0f*/, bool looping /*= false */ ) {
        i32 index = Random::Int( audioResources.GetCount() );
        AudioPlay( audioResources[ index ], pos, vel, volume, looping );
        return audioResources[ index ];
    }

}
