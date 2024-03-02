
#include "atto_core.h"
#include "atto_client.h"
#include "atto_mesh_generation.h"

namespace atto {

    f32 Core::GetDeltaTime() const {
        return deltaTime;
    }

    f64 Core::GetLastTime() const {
        return currentTime;
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

    char * Core::ResourceReadEntireFileIntoTransientMemory( const char * path, i64 * size ) {
        *size = ResourceGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocateTransient( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        ResourceReadEntireFile( path, data, (i32)( *size ) );

        return data;
    }

    DrawContext * Core::RenderGetDrawContext( i32 index, bool clear ) {
        DrawContext * d = &drawContexts[ index ];
        if( clear == true ) {
            ZeroStructPtr( d );
        }
        d->cameraProj = cameraProjection;
        d->screenProjection = screenProjection;
        d->mainSurfaceHeight = mainSurfaceHeight;
        d->mainSurfaceWidth = mainSurfaceWidth;
        d->mainAspect = mainSurfaceWidth / mainSurfaceHeight;
        d->cameraWidth = cameraWidth;
        d->cameraHeight = cameraHeight;
        d->viewport = viewport;

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
        drawList.Add( cmd );
    }

    void DrawContext::DrawRect( glm::vec2 bl, glm::vec2 tr, glm::vec4 colour /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = colour;
        cmd.proj = cameraProj;
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
        cmd.proj = cameraProj;

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

        cmd.rect.bl -= cameraPos;
        cmd.rect.tr -= cameraPos;
        cmd.rect.br -= cameraPos;
        cmd.rect.tl -= cameraPos;

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

    void DrawContext::DrawLine2D( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color /*= glm::vec4(1)*/ ) {
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
        cmd.line2D.p = screenProjection;
        cmd.line2D.p1 = points[ 0 ];
        cmd.line2D.p2 = points[ 1 ];
        cmd.line2D.p3 = points[ 2 ];
        cmd.line2D.p4 = points[ 3 ];

        drawList.Add( cmd );
    }


    void DrawContext::DrawLine2D_NDC( glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4 & color /*= glm::vec4( 1 ) */ ) {
        DrawLine2D( start, end, thicc, color );
        drawList.Last().line2D.p = glm::mat4( 1 );
    }

    void DrawContext::DrawTexture( TextureResource * texture, glm::vec2 center, f32 rot, glm::vec2 size, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXTURE;
        cmd.color = colour;
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

        cmd.texture.bl -= cameraPos;
        cmd.texture.tr -= cameraPos;
        cmd.texture.br -= cameraPos;
        cmd.texture.tl -= cameraPos;

        drawList.Add( cmd );
    }

    void DrawContext::DrawSprite( SpriteResource * sprite, i32 frameIndex, glm::vec2 center, f32 rot /*= 0.0f*/, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.spriteRes = sprite;
        cmd.sprite.frame = frameIndex;

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

        cmd.sprite.bl -= cameraPos;
        cmd.sprite.tr -= cameraPos;
        cmd.sprite.br -= cameraPos;
        cmd.sprite.tl -= cameraPos;

        const f32 textureWidth = (f32)sprite->textureResource->width; // @TODO(DECLAN): This '2' is a hack, we need to make an alpha border for each frame of animation, not the entire texture...
        const f32 frameWidth = (f32)sprite->frameWidth;

        cmd.sprite.blUV = glm::vec2( ( ( frameIndex * frameWidth ) / textureWidth ), 1.0f );
        cmd.sprite.trUV = glm::vec2( ( ( ( frameIndex + 1.0f ) * frameWidth ) / textureWidth ), 0.0f );
        cmd.sprite.tlUV = glm::vec2( cmd.sprite.blUV.x, cmd.sprite.trUV.y );
        cmd.sprite.brUV = glm::vec2( cmd.sprite.trUV.x, cmd.sprite.blUV.y );

        drawList.Add( cmd );
    }

    void DrawContext::DrawTextureBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        glm::vec2 dim = glm::vec2( texture->width, texture->height );
        DrawTexture( texture, bl + dim / 2.0f * size, 0.0f, size, colour );
    }

    void DrawContext::DrawText2D( FontHandle font, glm::vec2 bl, f32 fontSize, const char * text, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXT;
        cmd.color = colour;
        cmd.text.font = font;
        cmd.text.text = text;
        cmd.text.bl = bl;
        cmd.text.fontSize = fontSize;

        drawList.Add( cmd );
    }

    void DrawContext::DrawPlane( glm::vec3 center, glm::vec3 normal, glm::vec2 dim, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::PLANE;
        cmd.color = colour;
        cmd.plane.center = center;
        cmd.plane.normal = normal;
        cmd.plane.dim = dim;

        drawList.Add( cmd );
    }

    void DrawContext::DrawSphere( glm::vec3 center, f32 r, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPHERE;
        cmd.color = colour;
        cmd.sphere.center = center;
        cmd.sphere.r = r;

        drawList.Add( cmd );
    }

    void DrawContext::DrawBox( glm::vec3 min, glm::vec3 max, glm::vec4 colour ) {
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

    void DrawContext::DrawLine( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::LINE;
        cmd.color = colour;
        cmd.line.p1 = p1;
        cmd.line.p2 = p2;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TRIANGLE;
        cmd.color = colour;
        cmd.triangle.p1 = p1;
        cmd.triangle.p2 = p2;
        cmd.triangle.p3 = p3;

        drawList.Add( cmd );
    }

    void DrawContext::DrawTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, TextureResource * texture ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TRIANGLE;
        cmd.triangle.texture = texture;
        cmd.triangle.p1 = p1;
        cmd.triangle.p2 = p2;
        cmd.triangle.p3 = p3;
        cmd.triangle.uv1 = uv1;
        cmd.triangle.uv2 = uv2;
        cmd.triangle.uv3 = uv3;

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
        screenPos.y = mainSurfaceHeight - screenPos.y;

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

    void Core::NetConnect() {
        if( client != nullptr ) {
            client->Connect();
        }
    }

    bool Core::NetIsConnected() {
        if( client != nullptr ) {
            return client->IsConnected();
        }
        return false;
    }

    void Core::NetDisconnect() {
        if( client != nullptr ) {
            client->Disconnect();
        }
    }

    SmallString Core::NetStatusText() {
        if( client != nullptr ) {
            return client->StatusText();
        }

        return {};
    }

    u32 Core::NetGetPing() {
        return client->GetPing();
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
        return input.mousePosPixels;
    }

    glm::vec2 Core::InputMouseDeltaPixels() {
        return input.mouseDeltaPixels;
    }

    FrameInput & Core::InputGetFrameInput() {
        return input;
    }

    NetClient * Core::GetNetClient() {
        return client;
    }

    void Core::NetworkConnect() {
        client->Connect();
    }
    
    bool Core::NetworkIsConnected() {
        return client->IsConnected();
    }

    void Core::NetworkDisconnect() {
        client->Disconnect();
    }

    SmallString Core::NetworkGetStatusText() {
        return client->StatusText();
    }

    void Core::NetworkSend( const NetworkMessage & msg ) {
        Assert( msg.type != NetworkMessageType::NONE );
        client->Send( msg );
    }

    bool Core::NetworkRecieve( NetworkMessage & msg ) {
        return client->Recieve( msg );
    }

    u32 Core::NetworkGetPing() {
        return client->GetPing();
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

    LargeString SpriteResource::GetResourcePath() const {
        LargeString result = {};
        result.Add( "res/sprites/" );
        result.Add( spriteName );
        result.Add( '/' );
        result.Add( spriteName.GetFilePart() );
        result.Add( ".json" );
        return result;
    }

}


