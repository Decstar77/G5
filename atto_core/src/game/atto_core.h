#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"
#include "../shared/atto_math.h"
#include "../shared/atto_logging.h"
#include "../shared/atto_input.h"
#include "../shared/atto_network.h"
#include "../shared/atto_clock.h"
#include "../shared/atto_resources.h"
#include "../shared/atto_reflection.h"
#include "../shared/atto_jobs.h"

#include "atto_client.h"
namespace atto {
    class Core;
    class GameMode;
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
        SmallString     serverIp;

        static GameSettings CreateSensibleDefaults();

        REFLECT();
    };

    class SpriteAnimator {
    public:
        SpriteResource *    sprite;
        bool                animate;
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

    class GeometryFuncs {
    public:
        static void SortPointsIntoClockWiseOrder( glm::vec2 * points, i32 numPoints );
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

    struct AudioSpeaker;
    typedef ObjectHandle<AudioSpeaker> AudioSpeakerHandle;

    struct AudioSpeaker {
        AudioSpeakerHandle handle;
        void *  fmodChannel;
        AudioResource * source;
        AudioGroupResource * sourceGroup;
        f64 spawnTime;

        bool IsPlaying();
        void Stop();
    };

    enum class DrawCommandType {
        NONE = 0,
        CIRCLE,
        RECT,
        LINE2D,
        TRIANGLE,
        TEXTURE,
        SPRITE,
        TEXT,

        // 3D
        PLANE,
        SPHERE,
        BOX,
        LINE,
        TRIANGLE3D,
        MESH,
    };

    struct DrawCommand {
        DrawCommandType type;
        glm::vec4 color;
        glm::mat4 proj;
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
                TextureResource * textureRes;
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
                glm::vec2 p1;
                glm::vec2 p2;
                glm::vec2 p3;
                glm::vec2 uv1;
                glm::vec2 uv2;
                glm::vec2 uv3;
                TextureResource * texture;
            } triangle;
            struct {
                glm::vec3 p1;
                glm::vec3 p2;
                glm::vec3 p3;
                glm::vec2 uv1;
                glm::vec2 uv2;
                glm::vec2 uv3;
                TextureResource * texture;
            } triangle3D;
            struct {
                glm::mat4 m;
                StaticMeshResource * mesh;
                TextureResource * albedo;
            } mesh;
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
        friend class OpenglState;
        friend class VulkanState;
    public:
        void SetCameraPos( glm::vec2 pos );
        void DrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawRectScreen( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawLine_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color = glm::vec4( 1 ) );
        void DrawTriangle( glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTexture( TextureResource * texture, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureTL( TextureResource * texture, glm::vec2 tl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextureScreen( TextureResource * texture, glm::vec2 bl, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSprite( SpriteResource * sprite, i32 frameIndex, glm::vec2 center, f32 rot = 0.0f, glm::vec2 size = glm::vec2( 1 ), glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSpriteTile( SpriteResource * sprite, i32 tileX, i32 tileY, glm::vec2 center );
        void DrawTextCam( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTextScreen( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE, glm::vec4 colour = glm::vec4( 1 ) );

        void DrawPlane3D( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawSphere3D( glm::vec3 center, f32 r, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawBox3D( glm::vec3 min, glm::vec3 max, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawLine3D( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle3D( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour = glm::vec4( 1 ) );
        void DrawTriangle3D( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture );

        void DrawMesh( StaticMeshResource * mesh, glm::mat4 m, TextureResource * albedo = nullptr );

        glm::vec2           ScreenPosToWorldPos( glm::vec2 screenPos );
        glm::vec2           WorldPosToScreenPos( glm::vec2 worldPos );

        inline glm::vec2    GetMainSurfaceDims() const { return glm::vec2( mainSurfaceWidth, mainSurfaceHeight ); }
        inline f32          GetMainAspectRatio() const { return mainAspect; }

        void                SetCameraDims( f32 w, f32 h );
        inline f32          GetCameraWidth() const { return cameraWidth; }
        inline f32          GetCameraHeight() const { return cameraHeight; }
        inline glm::vec2    GetCameraDims() const { return glm::vec2( cameraWidth, cameraHeight ); }

    private:
        glm::mat4                   cameraView; // For 3D stuffies
        glm::vec2                   cameraPos;

        bool                        spriteOnly;
        i32                         spriteLayerCount;

        f32                         mainSurfaceWidth;
        f32                         mainSurfaceHeight;
        f32                         mainAspect;
        glm::vec4                   viewport;
        f32                         cameraWidth;
        f32                         cameraHeight;
        glm::mat4                   screenProjection;
        glm::mat4                   cameraProjection;
        GrowableList<DrawCommand>   drawList;
    };

    struct UIButton {
        glm::vec2 textPos;
        glm::vec2 center;
        glm::vec2 size;
        glm::vec4 color;
        SmallString text;
    };

    class Core {
        friend class OpenglState;
        friend class VulkanState;
    public:
        f32                                 GetDeltaTime() const;
        virtual f64                         GetTheCurrentTime() const = 0;
        Camera                              CreateDefaultCamera() const;

        inline GameSettings                 GetCurrentGameSettings() const { return theGameSettings; }

        void                                MoveToGameMode( GameMode * gameMode );
        
        bool                                LoadAllAssets();

        virtual FontHandle                  ResourceGetFont( const char * name ) = 0;

        virtual float                       FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA = TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V vA = TextAlignment_V::FONS_ALIGN_BASELINE ) = 0;

        DrawContext *                       RenderGetDrawContext( i32 index, bool clear = true );
        DrawContext *                       RenderGetSpriteDrawContext( i32 index );
        f32                                 RenderGetMainSurfaceWidth() const { return mainSurfaceWidth; }
        f32                                 RenderGetMainSurfaceHeight() const { return mainSurfaceHeight; }
        virtual void                        RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) = 0;

        virtual void                        AudioPlay( AudioResource * audioResource, AudioGroupResource * audioGroup ) = 0;
        virtual void                        AudioPlayRandomFromGroup( AudioGroupResource * audioGroup ) = 0;
        virtual void                        AudioSetListener( glm::vec2 pos ) = 0;

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

        virtual void *                      WindowGetHandle() = 0;
        virtual void                        WindowClose() = 0;
        virtual void                        WindowSetTitle( const char * title ) = 0;
        virtual void                        WindowSetVSync( bool value ) = 0;
        virtual bool                        WindowGetVSync() = 0;
        virtual bool                        WindowIsFullscreen() = 0;
        virtual bool                        WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) = 0;
        virtual bool                        WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) = 0;

        virtual void                        Run( int argc, char ** argv ) = 0;

        void                                NuklearUIInitialize();
        void                                NuklearUIScroll( f64 xoff, f64 yoff );
        void                                NuklearUIChar( u32 codepoint );
        void                                NuklearUIMouseButton( int button, int action, int mods );

        inline static Core *                theCore = nullptr;
        GameSettings                        theGameSettings = {};
        
    protected:
        GameMode *                          currentGameMode = nullptr;
        GameMode *                          nextGameMode = nullptr;

        f32                                 deltaTime = 0.0f;

        f32                                 mainSurfaceWidth;
        f32                                 mainSurfaceHeight;
        glm::mat4                           screenProjection;
        glm::vec4                           viewport;

        glm::vec2                           listenerPos;

        FrameInput                          input = {};
        FixedList<DrawContext, 8>           drawContexts = {};
    };

}
