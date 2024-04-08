
#include "atto_core.h"
#include "atto_client.h"
#include "atto_mesh_generation.h"

namespace atto {

    GameSettings GameSettings::CreateSensibleDefaults() {
        GameSettings settings = {};
        settings.basePath = "assets/";
        settings.windowStartPosX = -1;
        settings.windowStartPosY = -1;
        //settings.windowWidth = 16 * 115;
        //settings.windowHeight = 9 * 115;
        settings.windowWidth = 1280;
        settings.windowHeight = 720;
        //settings.windowWidth = 1920;
        //settings.windowHeight = 1050;
        settings.fullscreen = false;
        settings.vsync = true;
        settings.showDebug = true;
        settings.noAudio = false;
        settings.masterVolume = 0.25f;
        //settings.masterVolume = 1.0f;

        return settings;
    }

    f32 Core::GetDeltaTime() const {
        return deltaTime;
    }

    Camera Core::CreateDefaultCamera() const {
        Camera camera = {};
        camera.pos = glm::vec3( 0, 0, 0 );
        camera.dir = glm::vec3( 0, 0, -1 );
        camera.yfov = glm::radians( 45.0f );
        camera.zNear = 0.1f;
        camera.zFar = 250.0f;
        return camera;
    }

    void Core::MoveToGameMode( GameMode * gameMode ) {
        nextGameMode = gameMode;
    }

    char * Core::ResourceReadEntireTextFileIntoPermanentMemory( const char * path, i64 * size ) {
        *size = ResourceGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocatePermanent( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        ResourceReadEntireTextFile( path, data, (i32)( *size ) );

        return data;
    }

    char * Core::ResourceReadEntireTextFileIntoTransientMemory( const char * path, i64 * size ) {
        *size = ResourceGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocateTransient( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        ResourceReadEntireTextFile( path, data, (i32)( *size ) );

        return data;
    }


    char * Core::ResourceReadEntireBinaryFileIntoPermanentMemory( const char * path, i64 * size ) {
        *size = ResourceGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocatePermanent( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        ResourceReadEntireBinaryFile( path, data, (i32)( *size ) );

        return data;
    }

    char * Core::ResourceReadEntireBinaryFileIntoTransientMemory( const char * path, i64 * size ) {
        *size = ResourceGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocateTransient( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        ResourceReadEntireBinaryFile( path, data, (i32)( *size ) );

        return data;
    }

    DrawContext * Core::RenderGetDrawContext( i32 index, bool clear ) {
        DrawContext * d = &drawContexts[ index ];
        if( clear == true ) {
            ZeroStructPtr( d );
        }

        d->mainSurfaceHeight = mainSurfaceHeight;
        d->mainSurfaceWidth = mainSurfaceWidth;
        d->mainAspect = mainSurfaceWidth / mainSurfaceHeight;
        d->cameraWidth = 640;
        d->cameraHeight = 360;
        d->viewport = viewport;
        d->screenProjection = screenProjection;
        d->cameraProjection = glm::ortho( 0.0f, 640.0f, 0.0f, 360.0f, -1.0f, 1.0f );

        return d;
    }

    void DrawContext::SetCameraPos( glm::vec2 pos ) {
        cameraPos = pos;
    }

    void DrawContext::DrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::CIRCLE;
        cmd.color = colour;
        cmd.circle.c = pos;
        cmd.circle.r = radius;
        cmd.proj = cameraProjection;
        drawList.Add( cmd );
    }

    void DrawContext::DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = colour;
        cmd.proj = cameraProjection;
        cmd.rect.bl = bl;
        cmd.rect.br = glm::vec2( tr.x, bl.y );
        cmd.rect.tr = tr;
        cmd.rect.tl = glm::vec2( bl.x, tr.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawRect( glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4 & color /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = color;
        cmd.proj = cameraProjection;

        cmd.rect.bl = -dim / 2.0f;
        cmd.rect.tr = dim / 2.0f;
        cmd.rect.br = glm::vec2( cmd.rect.tr.x, cmd.rect.bl.y );
        cmd.rect.tl = glm::vec2( cmd.rect.bl.x, cmd.rect.tr.y );

        glm::mat2 rotationMatrix = glm::mat2( cos( rot ), -sin( rot ), sin( rot ), cos( rot ) );
        cmd.rect.bl = rotationMatrix * cmd.rect.bl;
        cmd.rect.tr = rotationMatrix * cmd.rect.tr;
        cmd.rect.br = rotationMatrix * cmd.rect.br;
        cmd.rect.tl = rotationMatrix * cmd.rect.tl;

        cmd.rect.bl += center;
        cmd.rect.tr += center;
        cmd.rect.br += center;
        cmd.rect.tl += center;

        drawList.Add( cmd );
    }

    void DrawContext::DrawRectScreen( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = colour;
        cmd.proj = screenProjection;
        cmd.rect.bl = bl;
        cmd.rect.br = glm::vec2( tr.x, bl.y );
        cmd.rect.tr = tr;
        cmd.rect.tl = glm::vec2( bl.x, tr.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawLine( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color /*= glm::vec4(1)*/ ) {
        glm::vec2 direction = glm::normalize( end - start );
        glm::vec2 perpendicular( direction.y, -direction.x );

        glm::vec2 points[] = {
            start + perpendicular * ( thicc / 2.0f ),
            start - perpendicular * ( thicc / 2.0f ),
            end + perpendicular * ( thicc / 2.0f ),
            end - perpendicular * ( thicc / 2.0f )
        };

        GeometryFuncs::SortPointsIntoClockWiseOrder( points, 4 );

        DrawCommand cmd = {};
        cmd.type = DrawCommandType::LINE2D;
        cmd.color = color;
        cmd.proj = cameraProjection;
        cmd.line2D.p1 = points[ 0 ];
        cmd.line2D.p2 = points[ 1 ];
        cmd.line2D.p3 = points[ 2 ];
        cmd.line2D.p4 = points[ 3 ];

        drawList.Add( cmd );
    }

    void DrawContext::DrawLine_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color /*= glm::vec4( 1 ) */ ) {
        DrawLine( start, end, thicc, color );
        drawList.Last().proj = glm::mat4( 1 );
    }

    void DrawContext::DrawTriangle( glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TRIANGLE;
        cmd.color = colour;
        cmd.proj = cameraProjection;

        cmd.triangle.p1 = p1;
        cmd.triangle.p2 = p2;
        cmd.triangle.p3 = p3;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTexture( TextureResource * texture, glm::vec2 center, f32 rot, glm::vec2 size, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXTURE;
        cmd.color = colour;
        cmd.proj = cameraProjection;
        cmd.texture.textureRes = texture;

        glm::vec2 dim = glm::vec2( texture->width, texture->height ) * size;
        cmd.texture.bl = -dim / 2.0f;
        cmd.texture.tr = dim / 2.0f;
        cmd.texture.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );
        cmd.texture.tl = glm::vec2( cmd.sprite.bl.x, cmd.sprite.tr.y );

        glm::mat2 rotationMatrix = glm::mat2( cos( rot ), -sin( rot ), sin( rot ), cos( rot ) );
        cmd.texture.bl = rotationMatrix * cmd.sprite.bl;
        cmd.texture.tr = rotationMatrix * cmd.sprite.tr;
        cmd.texture.br = rotationMatrix * cmd.sprite.br;
        cmd.texture.tl = rotationMatrix * cmd.sprite.tl;

        cmd.texture.bl += center;
        cmd.texture.tr += center;
        cmd.texture.br += center;
        cmd.texture.tl += center;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTextureBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXTURE;
        cmd.color = colour;
        cmd.proj = cameraProjection;
        cmd.texture.textureRes = texture;

        glm::vec2 dim = glm::vec2( texture->width, texture->height ) * size;
        cmd.texture.bl = bl;
        cmd.texture.tr = bl + dim;
        cmd.texture.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );
        cmd.texture.tl = glm::vec2( cmd.sprite.bl.x, cmd.sprite.tr.y );

        drawList.Add( cmd );
    }


    void DrawContext::DrawTextureTL( TextureResource * texture, glm::vec2 tl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXTURE;
        cmd.color = colour;
        cmd.proj = cameraProjection;
        cmd.texture.textureRes = texture;

        glm::vec2 dim = glm::vec2( texture->width, texture->height ) * size;
        cmd.texture.tl = tl;
        cmd.texture.bl = glm::vec2( tl.x, tl.y - dim.y );
        cmd.texture.tr = cmd.texture.bl + dim;
        cmd.texture.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawTextureScreen( TextureResource * texture, glm::vec2 bl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXTURE;
        cmd.color = colour;
        cmd.proj = screenProjection;
        cmd.texture.textureRes = texture;

        glm::vec2 dim = glm::vec2( texture->width, texture->height ) * size;
        cmd.texture.bl = bl;
        cmd.texture.tr = bl + dim;
        cmd.texture.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );
        cmd.texture.tl = glm::vec2( cmd.sprite.bl.x, cmd.sprite.tr.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawSprite( SpriteResource * sprite, i32 frameIndex, glm::vec2 center, f32 rot /*= 0.0f*/, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.spriteRes = sprite;
        cmd.proj = cameraProjection;

        glm::vec2 dim = glm::vec2( sprite->frameWidth, sprite->frameHeight ) * size;
        cmd.sprite.bl = -dim / 2.0f;
        cmd.sprite.tr = dim / 2.0f;
        cmd.sprite.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );
        cmd.sprite.tl = glm::vec2( cmd.sprite.bl.x, cmd.sprite.tr.y );

        glm::mat2 rotationMatrix = glm::mat2( cos( rot ), -sin( rot ), sin( rot ), cos( rot ) );
        cmd.sprite.bl = rotationMatrix * cmd.sprite.bl;
        cmd.sprite.tr = rotationMatrix * cmd.sprite.tr;
        cmd.sprite.br = rotationMatrix * cmd.sprite.br;
        cmd.sprite.tl = rotationMatrix * cmd.sprite.tl;

        cmd.sprite.bl += center;
        cmd.sprite.tr += center;
        cmd.sprite.br += center;
        cmd.sprite.tl += center;

        const f32 textureWidth = (f32)sprite->textureResource->width; // @TODO(DECLAN): This '2' is a hack, we need to make an alpha border for each frame of animation, not the entire texture...
        const f32 frameWidth = (f32)sprite->frameWidth;

        cmd.sprite.blUV = glm::vec2( ( ( frameIndex * frameWidth ) / textureWidth ), 1.0f );
        cmd.sprite.trUV = glm::vec2( ( ( ( frameIndex + 1.0f ) * frameWidth ) / textureWidth ), 0.0f );
        cmd.sprite.tlUV = glm::vec2( cmd.sprite.blUV.x, cmd.sprite.trUV.y );
        cmd.sprite.brUV = glm::vec2( cmd.sprite.trUV.x, cmd.sprite.blUV.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawSprite( SpriteResource * sprite, i32 tileX, i32 tileY, glm::vec2 center, f32 rot /*= 0.0f*/, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.spriteRes = sprite;
        cmd.proj = cameraProjection;

        glm::vec2 dim = glm::vec2( sprite->tileWidth, sprite->tileHeight ) * size;
        cmd.sprite.bl = -dim / 2.0f;
        cmd.sprite.tr = dim / 2.0f;
        cmd.sprite.br = glm::vec2( cmd.sprite.tr.x, cmd.sprite.bl.y );
        cmd.sprite.tl = glm::vec2( cmd.sprite.bl.x, cmd.sprite.tr.y );

        glm::mat2 rotationMatrix = glm::mat2( cos( rot ), -sin( rot ), sin( rot ), cos( rot ) );
        cmd.sprite.bl = rotationMatrix * cmd.sprite.bl;
        cmd.sprite.tr = rotationMatrix * cmd.sprite.tr;
        cmd.sprite.br = rotationMatrix * cmd.sprite.br;
        cmd.sprite.tl = rotationMatrix * cmd.sprite.tl;

        cmd.sprite.bl += center;
        cmd.sprite.tr += center;
        cmd.sprite.br += center;
        cmd.sprite.tl += center;

        const f32 textureWidth = (f32)sprite->textureResource->width;
        const f32 textureHeight = (f32)sprite->textureResource->height;
        const f32 tileWidth = (f32)sprite->tileWidth;
        const f32 tileHeight = (f32)sprite->tileHeight;
        const f32 tileXStart = (f32)( tileX * tileWidth ) / textureWidth;
        const f32 tileYStart = (f32)( tileY * tileHeight ) / textureHeight;
        const f32 tileXEnd = (f32)( ( tileX + 1 ) * tileWidth ) / textureWidth;
        const f32 tileYEnd = (f32)( ( tileY + 1 ) * tileHeight ) / textureHeight;

        cmd.sprite.blUV = glm::vec2( tileXStart, tileYEnd );
        cmd.sprite.trUV = glm::vec2( tileXEnd, tileYStart );
        cmd.sprite.tlUV = glm::vec2( cmd.sprite.blUV.x, cmd.sprite.trUV.y );
        cmd.sprite.brUV = glm::vec2( cmd.sprite.trUV.x, cmd.sprite.blUV.y );

        drawList.Add( cmd );
    }


    void DrawContext::DrawTextCam( FontHandle font, glm::vec2 tl, f32 fontSize, const char * text, TextAlignment_H hA, TextAlignment_V vA , glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXT;
        cmd.color = colour;
        cmd.proj = cameraProjection;
        cmd.text.font = font;
        cmd.text.text = text;
        cmd.text.bl = tl;
        cmd.text.fontSize = fontSize;
        cmd.text.align = (i32)hA | (i32)vA;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTextScreen( FontHandle font, glm::vec2 bl, f32 fontSize, const char * text, TextAlignment_H hA, TextAlignment_V vA, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXT;
        cmd.color = colour;
        cmd.proj = screenProjection;
        cmd.text.font = font;
        cmd.text.text = text;
        cmd.text.bl = bl;
        cmd.text.fontSize = fontSize;
        cmd.text.align = (i32)hA | (i32)vA;

        drawList.Add( cmd );
    }

    void DrawContext::DrawPlane3D( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::PLANE;
        cmd.color = colour;
        cmd.plane.center = center;
        cmd.plane.normal = normal;
        cmd.plane.dim = dim;

        drawList.Add( cmd );
    }

    void DrawContext::DrawSphere3D( glm::vec3 center, f32 r, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPHERE;
        cmd.color = colour;
        cmd.sphere.center = center;
        cmd.sphere.r = r;

        drawList.Add( cmd );
    }

    void DrawContext::DrawBox3D( glm::vec3 min, glm::vec3 max, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::BOX;
        cmd.color = colour;

        // We are a 1x1x1 box so compute the scale pos and rot
        glm::mat4 scale = glm::scale( glm::mat4( 1 ), max - min );
        glm::mat4 pos = glm::translate( glm::mat4( 1 ), ( max + min ) / 2.0f );
        glm::mat4 rot = glm::mat4( 1 );
        cmd.box.m = pos * rot * scale;

        drawList.Add( cmd );
    }

    void DrawContext::DrawLine3D( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::LINE;
        cmd.color = colour;
        cmd.line.p1 = p1;
        cmd.line.p2 = p2;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTriangle3D( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TRIANGLE3D;
        cmd.color = colour;
        cmd.triangle3D.p1 = p1;
        cmd.triangle3D.p2 = p2;
        cmd.triangle3D.p3 = p3;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTriangle3D( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TRIANGLE3D;
        cmd.triangle3D.texture = texture;
        cmd.triangle3D.p1 = p1;
        cmd.triangle3D.p2 = p2;
        cmd.triangle3D.p3 = p3;
        cmd.triangle3D.uv1 = uv1;
        cmd.triangle3D.uv2 = uv2;
        cmd.triangle3D.uv3 = uv3;

        drawList.Add( cmd );
    }

    void DrawContext::DrawMesh( StaticMeshResource * mesh, glm::mat4 m, TextureResource * albedo ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::MESH;
        cmd.mesh.mesh = mesh;
        cmd.mesh.m = m;
        cmd.mesh.albedo = albedo;

        drawList.Add( cmd );
    }

    glm::vec2 DrawContext::ScreenPosToWorldPos( glm::vec2 screenPos ) {
        // @NOTE: Convert to [ 0, 1] not NDC[-1, 1] because 
        // @NOTE: we're doing a small optimization here by not doing the inverse of the camera matrix
        // @NOTE: but instead just using the camera width and height

        f32 l = viewport.x;
        f32 r = viewport.x + viewport.z;
        f32 nx = ( screenPos.x - l ) / ( r - l );

        f32 b = viewport.y;
        f32 t = viewport.y + viewport.w;
        f32 ny = ( screenPos.y - b ) / ( t - b );

        f32 wx = nx * cameraWidth;
        f32 wy = ny * cameraHeight;

        return glm::vec2( wx, wy ) + cameraPos;
    }

    glm::vec2 DrawContext::WorldPosToScreenPos( glm::vec2 worldPos ) {
        // Undo the translation by subtracting camera position
        glm::vec2 localPos = worldPos - cameraPos;

        // Calculate the normalized device coordinates (NDC) in the range [0, 1]
        float nx = localPos.x / cameraWidth;
        float ny = localPos.y / cameraHeight;

        // Undo the normalization to get coordinates in viewport space
        float l = viewport.x;
        float r = viewport.x + viewport.z;
        float screenX = l + nx * (r - l);

        float b = viewport.y;
        float t = viewport.y + viewport.w;
        float screenY = b + ny * (t - b);

        // Construct and return the screen position
        return glm::vec2(screenX, screenY);
    }

    void DrawContext::SetCameraDims( f32 w, f32 h ) {
        cameraWidth = w;
        cameraHeight = h;
        cameraProjection = glm::ortho( 0.0f, cameraWidth, 0.0f, cameraHeight, -1.0f, 1.0f );
    }

    void * Core::MemoryAllocatePermanent( u64 bytes ) {
        thePermanentMemoryMutex.lock();

        AssertMsg( thePermanentMemoryCurrent + bytes < thePermanentMemorySize, "Permanent memory overflow" );
        if( thePermanentMemoryCurrent + bytes < thePermanentMemorySize ) {
            void * result = thePermanentMemory + thePermanentMemoryCurrent;
            thePermanentMemoryCurrent += bytes;
            thePermanentMemoryMutex.unlock();

            memset( result, 0, bytes );

            return result;
        }

        thePermanentMemoryMutex.unlock();

        return nullptr;
    }

    bool Core::InputKeyDown( KeyCode keyCode ) {
        return input.keys[ keyCode ];
    }

    bool Core::InputKeyUp( KeyCode keyCode ) {
        return !input.keys[ keyCode ];
    }

    bool Core::InputKeyJustPressed( KeyCode keyCode ) {
        return input.keys[ keyCode ] && !input.lastKeys[ keyCode ];
    }

    bool Core::InputKeyJustReleased( KeyCode keyCode ) {
        return !input.keys[ keyCode ] && input.lastKeys[ keyCode ];
    }

    bool Core::InputMouseButtonDown( MouseButton button ) {
        return input.mouseButtons[ button ];
    }

    bool Core::InputMouseButtonUp( MouseButton button ) {
        return !input.mouseButtons[ button ];
    }

    bool Core::InputMouseButtonJustPressed( MouseButton button ) {
        return input.mouseButtons[ button ] && !input.lastMouseButtons[ button ];
    }

    bool Core::InputMouseButtonJustReleased( MouseButton button ) {
        return !input.mouseButtons[ button ] && input.lastMouseButtons[ button ];
    }

    bool Core::InputMouseHasMoved() {
        return input.lastMousePosPixels - input.mousePosPixels != glm::vec2( 0 );
    }

    glm::vec2 Core::InputMousePosNDC() {
        return ( input.mousePosPixels / glm::vec2( mainSurfaceWidth, -mainSurfaceHeight ) * 2.0f ) - glm::vec2( 1, -1 );
    }

    glm::vec2 Core::InputMousePosPixels() {
        glm::vec2 screenPos = input.mousePosPixels;
        screenPos.y = mainSurfaceHeight - screenPos.y;
        return screenPos;
    }

    glm::vec2 Core::InputMouseDeltaPixels() {
        return input.mouseDeltaPixels;
    }

    f32 Core::InputMouseWheelDelta() {
        return input.mouseWheelDelta.y;
    }

    FrameInput & Core::InputGetFrameInput() {
        return input;
    }

    void * Core::MemoryAllocateTransient( u64 bytes ) {
        theTransientMemoryMutex.lock();

        AssertMsg( theTransientMemoryCurrent + bytes < theTransientMemorySize, "Transient memory overflow" );
        if( theTransientMemoryCurrent + bytes < theTransientMemorySize ) {
            void * result = theTransientMemory + theTransientMemoryCurrent;
            theTransientMemoryCurrent += bytes;
            theTransientMemoryMutex.unlock();

            memset( result, 0, bytes );

            return result;
        }

        theTransientMemoryMutex.unlock();

        return nullptr;
    }

    void Core::MemoryMakePermanent( u64 bytes ) {
        thePermanentMemory = (u8 *)malloc( bytes );
        thePermanentMemorySize = bytes;
    }

    void Core::MemoryClearPermanent() {
        thePermanentMemoryMutex.lock();

    #if ATTO_DEBUG
        memset( thePermanentMemory, 0, thePermanentMemorySize );
    #endif

        thePermanentMemoryCurrent = 0;
        thePermanentMemoryMutex.unlock();
    }

    void Core::MemoryMakeTransient( u64 bytes ) {
        theTransientMemory = (u8 *)malloc( bytes );
        theTransientMemorySize = bytes;
    }

    void Core::MemoryClearTransient() {
        theTransientMemoryMutex.lock();

    #if ATTO_DEBUG
        memset( theTransientMemory, 0, theTransientMemorySize );
    #endif

        theTransientMemoryCurrent = 0;
        theTransientMemoryMutex.unlock();
    }

    //glm::vec2 Camera::ScreenPointToWorld( glm::vec2 screen ) {
    //    glm::vec3 win(screen.x, screen.y, 0);
    //    glm::vec3 world = glm::unProject(win, v, p, viewport);
    //
    //    return glm::vec2(world.x, world.y);
    //}
    //
    //glm::vec2 Camera::WorldPointToScreen(glm::vec2 world){
    //    glm::vec3 win = glm::project(glm::vec3(world.x, world.y, 0), v, p, viewport);
    //
    //    //glm::mat4 t = p * v;
    //    //glm::vec4 win = t * glm::vec4( world.x, world.y, 0, 1 );
    //    //glm::vec4 ww = win / win.w;
    //    //// Map to range 0 - 1
    //    //glm::vec4 win2 = ( ww + glm::vec4( 1.0f ) ) / glm::vec4( 2.0f );
    //    //win2.y = 1.0f - win2.y;
    //
    //    //win.x = win2.x * mainSurfaceWidth;
    //    //win.y = win2.y * mainSurfaceHeight;
    //
    //    return glm::vec2(win.x, win.y);
    //}

    void StaticMeshData::Free() {
        if( vertices != nullptr ) {
            free( vertices );
        }
        if( indices != nullptr ) {
            free( indices );
        }
    }

    void SpriteResource::GetUVForTile(i32 tileX, i32 tileY, glm::vec2 & bl, glm::vec2 & tr) const {
        const f32 textureWidth = (f32)textureResource->width;
        const f32 textureHeight = (f32)textureResource->height;
        const f32 tileXStart = (f32)(tileX * tileWidth) / textureWidth;
        const f32 tileYStart = (f32)(tileY * tileHeight) / textureHeight;
        const f32 tileXEnd = (f32)((tileX + 1) * tileWidth) / textureWidth;
        const f32 tileYEnd = (f32)((tileY + 1) * tileHeight) / textureHeight;

        bl = glm::vec2(tileXStart, tileYEnd);
        tr = glm::vec2(tileXEnd, tileYStart);
    }

    LargeString SpriteResource::GetResourcePath() const {
        return spriteName;
    }


    void SpriteAnimator::SetFrameRate( f32 fps ) {
        frameDuration = 1.0f / fps;
    }

    bool SpriteAnimator::SetSpriteIfDifferent( SpriteResource * sprite, bool loops ) {
        if( this->sprite != sprite ) {
            this->sprite = sprite;
            if ( sprite == nullptr ) {
                return true;
            }

            SetFrameRate( (f32)sprite->frameRate );
            animate = true;
            frameIndex = 0;
            frameTimer = 0;
            loopCount = 0;
            this->loops = loops;
            color = glm::vec4( 1 );

            return true;
        }
        return false;
    }

    void SpriteAnimator::Update( Core * core, f32 dt ) {
        if ( animate == true && sprite != nullptr && sprite->frameCount > 1 ) {
            if( frameTimer == 0.0f && frameIndex == 0 ) { // test to see if we have just started and neeed to play something
                TestFrameActuations( core );
            }

            frameTimer += dt;
            if( frameTimer >= frameDuration ) {
                frameTimer -= frameDuration;
                if( frameDelaySkip == 0 ) {
                    frameIndex++;

                    TestFrameActuations( core );

                    if( frameIndex >= sprite->frameCount ) {
                        if( loops == true ) {
                            frameIndex = 0;
                            loopCount++;
                        }
                        else {
                            if( frameIndex >= sprite->frameCount ) {
                                frameIndex = sprite->frameCount - 1;
                                loopCount = 1;
                            }
                        }
                    }
                }
                else {
                    frameDelaySkip--;
                }
            }
        }
    }

    void SpriteAnimator::TestFrameActuations( Core * core ) {
        const i32 frameActuationCount = sprite->frameActuations.GetCount();
        for( i32 frameActuationIndex = 0; frameActuationIndex < frameActuationCount; frameActuationIndex++ ) {
            SpriteActuation & frameActuation = sprite->frameActuations[ frameActuationIndex ];
            if( frameActuation.frameIndex == frameIndex ) {
                if( frameActuation.audioResources.GetCount() > 0 ) {
                    core->AudioPlayRandom( frameActuation.audioResources );
                }
            }
        }
    }

    LargeString Resource::GetShortName() const {
        LargeString result = {};
        result.Add( name.GetFilePart() );
        return result;
    }

}


