
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

    DrawContext * Core::RenderGetDrawContext( i32 index, bool clear ) {
        DrawContext * d = &drawContexts[ index ];
        if ( clear == true ) { 
            ZeroStructPtr( d );
        }
        d->screenProjection = screenProjection;
        d->mainSurfaceHeight = mainSurfaceHeight;
        d->mainSurfaceWidth = mainSurfaceWidth;
        d->mainAspect = mainSurfaceWidth / mainSurfaceHeight;
        return d;
    }

    void DrawContext::SetCamera( glm::mat4 v, f32 yfov, f32 zNear, f32 zFar ) {
        cameraView = v;
        cameraProj = glm::perspective( glm::radians( yfov ), mainAspect, zNear, zFar );
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

    void DrawContext::DrawSprite( TextureResource * texture, glm::vec2 center, f32 rot, glm::vec2 size, glm::vec4 colour ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.textureRes = texture;

        glm::vec2 dim = glm::vec2( texture->width, texture->height ) * size;
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

    void DrawContext::DrawSpriteBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        glm::vec2 dim = glm::vec2( texture->width, texture->height );
        DrawSprite( texture, bl + dim / 2.0f * size, 0.0f, size, colour );
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

    void DrawContext::DrawSphere( glm::vec3 center, f32 r, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPHERE;
        cmd.color = colour;
        cmd.sphere.center = center;
        cmd.sphere.r = r;

        drawList.Add( cmd );
    }

    void DrawContext::DrawLine( glm::vec3 p1, glm::vec3 p2, f32 thicc, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
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

}


