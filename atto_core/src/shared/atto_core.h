#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_math.h"
#include "atto_logging.h"
#include "atto_input.h"
#include "atto_network.h"
#include "atto_clock.h"

#include "atto_reflection.h"
#include "enki_task_scheduler.h"

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
        bool            usePackedAssets;
        SmallString     basePath;
        f32             masterVolume;
        
        static GameSettings CreateSensibleDefaults();

        REFLECT();
    };

    class Resource {
    public:
        u32             id;
        LargeString     name;

        LargeString     GetShortName() const;
    };

    class TextureResource : public Resource {
    public:
        u32     handle;
        i32     width;
        i32     height;
        i32     channels;
        bool    hasMips;
        bool    hasAnti;

        inline i32 GetByteSize() const { 
            i32 size = width * height * channels * 1;
            return size;
        }

        REFLECT();
    };

    class AudioResource : public Resource {
    public:
        bool        is2D;
        bool        is3D;
        f32         minDist;
        f32         maxDist;
        i32         audioSize;

        REFLECT();
    };

    struct SpriteActuation {
        i32                              frameIndex;
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

        void                            GetUVForTile( i32 tileX, i32 tileY, glm::vec2 & bl, glm::vec2 & tr ) const;
        LargeString                     GetResourcePath() const;

        REFLECT();
    };

    class SpriteAnimator {
    public:
        SpriteResource *    sprite;
        i32                 frameIndex;
        f32                 frameTimer;
        f32                 frameDuration;
        i32                 loopCount;
        i32                 frameDelaySkip;
        bool                loops;
        glm::vec4           color;

    public:
        void                SetFrameRate( f32 fps );
        bool                SetSpriteIfDifferent( SpriteResource * sprite, bool loops );
        void                Update( Core * core, f32 dt );
        void                TestFrameActuations( Core * core );

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
                    i32 align;
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

    enum class TextAlignment_H {
        FONS_ALIGN_LEFT = 1 << 0, // Default
        FONS_ALIGN_CENTER = 1 << 1,
        FONS_ALIGN_RIGHT = 1 << 2,
    };

    enum class TextAlignment_V {
        FONS_ALIGN_TOP = 1 << 3,
        FONS_ALIGN_MIDDLE = 1 << 4,
        FONS_ALIGN_BOTTOM = 1 << 5,
        FONS_ALIGN_BASELINE = 1 << 6, // Default
    };

    class DrawContext {
        friend class Core;
        friend class WindowsCore;
    public:
        void SetCameraPos( glm::vec2 pos );
        void DrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawRectScreen( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine2D( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine2D_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawTexture( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureTL( TextureResource * texture, glm::vec2 tl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureScreen( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSprite( SpriteResource * sprite, i32 frameIndex, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSprite( SpriteResource * sprite, i32 tileX, i32 tileY, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextCam( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawText2D( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawPlane( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSphere( glm::vec3 center, f32 r, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawBox( glm::vec3 min, glm::vec3 max, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture );

        void DrawMesh( StaticMeshResource * mesh, glm::mat4 m, TextureResource * albedo = nullptr );

        glm::vec2           ScreenPosToWorldPos( glm::vec2 screenPos );

        inline glm::vec2    GetMainSurfaceDims() const { return glm::vec2( mainSurfaceWidth, mainSurfaceHeight ); }
        inline f32          GetMainAspectRatio() const { return mainAspect; }

        void                SetCameraDims( f32 w, f32 h );
        inline f32          GetCameraWidth() const { return cameraWidth; }
        inline f32          GetCameraHeight() const { return cameraHeight; }
        inline glm::vec2    GetCameraDims() const { return glm::vec2( cameraWidth, cameraHeight ); }

    private:
        glm::mat4       cameraView; // For 3D stuffies
        glm::vec2       cameraPos;

        f32             mainSurfaceWidth;
        f32             mainSurfaceHeight;
        f32             mainAspect;
        glm::vec4       viewport;
        f32             cameraWidth;
        f32             cameraHeight;
        glm::mat4       screenProjection;
        glm::mat4       cameraProjection;
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
        
        virtual TextureResource *           ResourceGetAndCreateTexture( const char * name, bool genMips, bool genAnti ) = 0;
        virtual TextureResource *           ResourceGetAndLoadTexture( const char * name ) = 0;

        virtual AudioResource *             ResourceGetAndCreateAudio( const char * name, bool is2D, bool is3D, f32 minDist, f32 maxDist ) = 0;
        virtual AudioResource *             ResourceGetAndLoadAudio( const char * name ) = 0;

        virtual SpriteResource *            ResourceGetAndCreateSprite( const char * spriteName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) = 0;
        virtual SpriteResource *            ResourceGetAndLoadSprite( const char * spriteName ) = 0;
        virtual SpriteResource *            ResourceGetLoadedSprite( i64 spriteId ) = 0; 

        virtual FontHandle                  ResourceGetFont( const char * name ) = 0;
        
        virtual void                        ResourceReadEntireTextFile( const char * path, char * data, i32 maxLen ) = 0;
        virtual void                        ResourceWriteEntireTextFile( const char * path, const char * data ) = 0;
        virtual void                        ResourceReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) = 0;
        virtual void                        ResourceWriteEntireBinaryFile( const char * path, const char * data, i32 size ) = 0;
        virtual i64                         ResourceGetFileSize( const char * path ) = 0;
        
        char *                              ResourceReadEntireTextFileIntoPermanentMemory( const char * path, i64 * size );
        char *                              ResourceReadEntireTextFileIntoTransientMemory( const char * path, i64 * size );
        char *                              ResourceReadEntireBinaryFileIntoPermanentMemory( const char * path, i64 * size );
        char *                              ResourceReadEntireBinaryFileIntoTransientMemory( const char * path, i64 * size );

        template< typename _type_ >
        void                                ResourceWriteTextRefl( const _type_ * obj, const char * path );
        template< typename _type_>
        bool                                ResourceReadTextRefl( _type_ * obj, const char * path );
        template< typename _type_ >
        void                                ResourceWriteBinaryRefl( const _type_ * obj, const char * path );
        template< typename _type_>
        bool                                ResourceReadBinaryRefl( _type_ * obj, const char * path );

        virtual float                       FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE ) = 0;

        DrawContext *                       RenderGetDrawContext( i32 index, bool clear = true );
        f32                                 RenderGetMainSurfaceWidth() const { return mainSurfaceWidth; }
        f32                                 RenderGetMainSurfaceHeight() const { return mainSurfaceHeight; }
        virtual void                        RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) = 0;

        virtual AudioSpeaker                AudioPlay( AudioResource * audioResource, glm::vec2 * pos = nullptr ) = 0;
        virtual void                        AudioSetListener( glm::vec2 pos ) = 0;
        template<i32 capcity>
        AudioResource *                     AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, glm::vec2 * pos = nullptr );
        template<typename... args>
        AudioResource *                     AudioPlayRandom( glm::vec2 * pos, args... audioResources );

        void                                NetConnect();
        bool                                NetIsConnected();
        void                                NetDisconnect();
        SmallString                         NetStatusText();
        u32                                 NetGetPing();

        BinaryBlob                          CreateBinaryBlob( i32 blobSize );

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
        f32                                 InputMouseWheelDelta();
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
        virtual bool                        WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) = 0;
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

    #if ATTO_EDITOR
        inline static Core *                EditorOnly_GetCore() { return theCore; }
        virtual void                        EditorOnly_SaveLoadedResourcesToBinary() = 0;
    #endif

        enki::TaskScheduler                 taskScheduler = {}; // @TODO: Make private

    protected:
        inline static Core *                theCore = nullptr;
        GameSettings                        theGameSettings = {};
        LoggingState                        logger = {};
        FixedList<DrawContext, 8>           drawContexts = {};
        FrameInput                          input = {};
        UIState                             uiState = {};

        GameMode *                          currentGameMode = nullptr;
        GameMode *                          nextGameMode = nullptr;
        Editor *                            editor = nullptr;
        NetClient *                         client = nullptr;

        f32                                 deltaTime = 0.0f;

        f32                                 mainSurfaceWidth;
        f32                                 mainSurfaceHeight;
        glm::mat4                           screenProjection;
        glm::vec4                           viewport;

        glm::vec2           listenerPos;

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
    void Core::ResourceWriteTextRefl( const _type_ * obj, const char * path ) {
        TypeDescriptor * settingsType = TypeResolver<_type_>::get();
        nlohmann::json j = settingsType->JSON_Write( obj );
        ResourceWriteEntireTextFile( path, j.dump().c_str() );
    }

    template< typename _type_>
    bool Core::ResourceReadTextRefl( _type_ * obj, const char * path ) {
        i64 fileSize;
        char * data = ResourceReadEntireTextFileIntoTransientMemory( path, &fileSize );
        if( data != nullptr ) {
            nlohmann::json j = nlohmann::json::parse( data );
            TypeDescriptor * settingsType = TypeResolver<_type_>::get();
            settingsType->JSON_Read( j, obj );
            return true;
        }
        else {
            LogOutput( LogLevel::ERR, "ResourceReadTextRefl :: Could not load %s", path );
        }

        return false;
    }

    template< typename _type_ >
    void Core::ResourceWriteBinaryRefl( const _type_ * obj, const char * path ) {
        TypeDescriptor * settingsType = TypeResolver<_type_>::get();
        const i32 tempSize = Megabytes( 100 );
        BinaryBlob blob = {};
        blob.Create( (byte*)MemoryAllocateTransient( tempSize ), tempSize );
        settingsType->Binary_Write( obj, blob );
        ResourceWriteEntireBinaryFile( path, (char*)blob.buffer, blob.current );
    }

    template< typename _type_>
    bool Core::ResourceReadBinaryRefl( _type_ * obj, const char * path ) {
        i64 fileSize;
        char * data = ResourceReadEntireTextFileIntoTransientMemory( path, &fileSize );
        if( data != nullptr ) {
            TypeDescriptor * settingsType = TypeResolver<_type_>::get();
            BinaryBlob blob = {};
            blob.Create( data, (i32)fileSize );
            settingsType->Binary_Read( obj );
            return true;
        }
        
        return false;
    }

    template<typename... args>
    AudioResource * Core::AudioPlayRandom( glm::vec2 * pos, args... audioResources ) {
        AudioResource * audioResourceArray[] = { audioResources... };
        i32 index = Random::Int( sizeof...( audioResources ) );
        AudioPlay( audioResourceArray[ index ], pos );
        return audioResourceArray[ index ];
    }

    template<i32 capcity>
    AudioResource * Core::AudioPlayRandom( const FixedList<AudioResource *, capcity> & audioResources, glm::vec2 * pos ) {
        i32 index = Random::Int( audioResources.GetCount() );
        AudioPlay( audioResources[ index ], pos );
        return audioResources[ index ];
    }


}
