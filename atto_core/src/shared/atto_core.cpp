
#include "atto_core.h"
#include "atto_client.h"

namespace atto {

    f32 Core::GetDeltaTime() const {
        return deltaTime;
    }

    f64 Core::GetLastTime() const {
        return currentTime;
    }

    f32 Core::FontGetWidth( FontResource * font, const char * text ) {
        f32 width = 0;

        for( i32 i = 0; text[ i ] != '\0'; i++ ) {
            width += (f32)( font->chars[ text[ i ] ].advance >> 6 );
        }

        return width;
    }

    f32 Core::FontGetHeight( FontResource * font ) {
        return (f32)font->fontSize;
    }

    void Core::UIBegin() {
        uiState.buttons.Clear();
    }

    bool Core::UIPushButton( const char * text, glm::vec2 center, glm::vec4 color /*= glm::vec4( 1 ) */ ) {
        glm::vec2 screenScale = glm::vec2( viewport.z, viewport.w );
        f32 width = FontGetWidth( arialFont, text );
        f32 height = FontGetHeight( arialFont );
        glm::vec2 pad = glm::vec2( 8.0f, 8.0f );

        UIButton btn = {};
        btn.text = text;
        btn.center = center * screenScale;
        btn.size = glm::vec2( width, height ) + pad;
        btn.textPos = btn.center - glm::vec2( width / 2.0f, -height / 2.0f );
        btn.color = color;

        BoxBounds bb = {};
        bb.CreateFromCenterSize( btn.center, btn.size );
        if( bb.Contains( input.mousePosPixels ) ) {
            btn.color = color * 1.3f;
            if( InputMouseButtonDown( MOUSE_BUTTON_1 ) ) {
                btn.color = color * 1.6f;
            }
            if( InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) ) {
                return true;
            }
        }

        uiState.buttons.Add( btn );

        return false;
    }

    void Core::UIEndAndRender() {
        const i32 buttonCount = uiState.buttons.GetCount();
        for( i32 i = 0; i < buttonCount; ++i ) {
            UIButton & btn = uiState.buttons[ i ];
            RenderDrawRect( btn.center, btn.size, 0.0f, btn.color );
            RenderDrawText( arialFont, btn.textPos, btn.text.GetCStr() );
        }
    }

    glm::vec2 Core::WorldPosToScreenPos( glm::vec2 world ) {
        // @NOTE: Convert to [ 0, 1] not NDC[-1, 1] because 
        // @NOTE: we're doing a small optimization here by not doing the inverse of the camera matrix
        // @NOTE: but instead just using the camera width and height

        f32 l = viewport.x;
        f32 r = viewport.x + viewport.z;
        f32 nx = world.x / cameraWidth;
        f32 sx = l + nx * ( r - l );

        f32 b = viewport.y;
        f32 t = viewport.y + viewport.w;
        f32 ny = world.y / cameraHeight;
        f32 sy = b + ny * ( t - b );

        sy = mainSurfaceHeight - sy;

        return glm::vec2( sx, sy );
    }

    glm::vec2 Core::ScreenPosToWorldPos( glm::vec2 screenPos ) {
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

        return glm::vec2( wx, wy );
    }

    void Core::RenderDrawCircle( glm::vec2 pos, f32 radius, glm::vec4 colour /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::CIRCLE;
        cmd.color = colour;
        cmd.circle.c = pos;
        cmd.circle.r = radius;
        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawRect(glm::vec2 bl, glm::vec2 tr, glm::vec4 colour /*= glm::vec4(1)*/) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = colour;
        cmd.rect.bl = bl;
        cmd.rect.br = glm::vec2(tr.x, bl.y);
        cmd.rect.tr = tr;
        cmd.rect.tl = glm::vec2(bl.x, tr.y);
        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawRect(glm::vec2 center, glm::vec2 dim, f32 rot, const glm::vec4& color /*= glm::vec4(1)*/) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::RECT;
        cmd.color = color;

        cmd.rect.bl = -dim / 2.0f;
        cmd.rect.tr = dim / 2.0f;
        cmd.rect.br = glm::vec2(cmd.rect.tr.x, cmd.rect.bl.y);
        cmd.rect.tl = glm::vec2(cmd.rect.bl.x, cmd.rect.tr.y);

        glm::mat2 rotationMatrix = glm::mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
        cmd.rect.bl = rotationMatrix * cmd.rect.bl;
        cmd.rect.tr = rotationMatrix * cmd.rect.tr;
        cmd.rect.br = rotationMatrix * cmd.rect.br;
        cmd.rect.tl = rotationMatrix * cmd.rect.tl;

        cmd.rect.bl += center;
        cmd.rect.tr += center;
        cmd.rect.br += center;
        cmd.rect.tl += center;

        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawLine(glm::vec2 start, glm::vec2 end, f32 thicc, const glm::vec4& color /*= glm::vec4(1)*/) {
        //glm::vec2 direction = glm::normalize( end - start );
        //glm::vec2 perpendicular( direction.y, -direction.x );

        //glm::vec2 points[] = {
        //    start + perpendicular * ( thicc / 2.0f ),
        //    start - perpendicular * ( thicc / 2.0f ),
        //    end + perpendicular * ( thicc / 2.0f ),
        //    end - perpendicular * ( thicc / 2.0f )
        //};

        //Geometry::SortPointsIntoClockWiseOrder( points, 4 );

        //DrawCommand cmd = {};
        //cmd.type = DrawCommandType::LINE;
        //cmd.color = color;
        //cmd.p1 = points[ 0 ];
        //cmd.p2 = points[ 1 ];
        //cmd.p3 = points[ 2 ];
        //cmd.p4 = points[ 3 ];


        //drawCommands.drawList.Add( cmd );
    }

    void Core::RenderDrawSprite(TextureResource* texture, glm::vec2 center, f32 rot, glm::vec2 size, glm::vec4 colour) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.textureRes = texture;
        
        glm::vec2 dim = glm::vec2(texture->width, texture->height) * size;
        cmd.rect.bl = -dim / 2.0f;
        cmd.rect.tr = dim / 2.0f;
        cmd.rect.br = glm::vec2(cmd.rect.tr.x, cmd.rect.bl.y);
        cmd.rect.tl = glm::vec2(cmd.rect.bl.x, cmd.rect.tr.y);

        glm::mat2 rotationMatrix = glm::mat2( cos( rot ), -sin( rot ), sin( rot ), cos( rot ) );
        cmd.rect.bl = rotationMatrix * cmd.rect.bl;
        cmd.rect.tr = rotationMatrix * cmd.rect.tr;
        cmd.rect.br = rotationMatrix * cmd.rect.br;
        cmd.rect.tl = rotationMatrix * cmd.rect.tl;

        cmd.rect.bl += center;
        cmd.rect.tr += center;
        cmd.rect.br += center;
        cmd.rect.tl += center;

        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawSpriteBL( TextureResource * texture, glm::vec2 bl, glm::vec2 size /*= glm::vec2( 1 )*/, glm::vec4 colour /*= glm::vec4( 1 ) */ ) {
        glm::vec2 dim = glm::vec2( texture->width, texture->height );
        RenderDrawSprite( texture, bl + dim / 2.0f * size, 0.0f, size, colour );
    }

    void Core::RenderDrawText( FontResource * font, glm::vec2 bl, const char * text, glm::vec4 colour /*= glm::vec4(1)*/ ) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXT;
        cmd.color = colour;
        cmd.text.bl = bl;
        cmd.text.text = text;
        cmd.text.proj = screenProjection;
        cmd.text.fontRes = font;
        drawCommands.drawList.Add(cmd);
    }

    void Core::NetConnect() {
        if (client != nullptr) {
            client->Connect();
        }
    }

    bool Core::NetIsConnected() {
        if (client != nullptr) {
            return client->IsConnected();
        }
        return false;
    }

    void Core::NetDisconnect() {
        if (client != nullptr) {
            client->Disconnect();
        }
    }

    SmallString Core::NetStatusText() {
        if (client != nullptr) {
            return client->StatusText();
        }

        return {};
    }

    u32 Core::NetGetPing() {
        return client->GetPing();
    }

    void* Core::MemoryAllocatePermanent(u64 bytes) {
        thePermanentMemoryMutex.lock();

        AssertMsg(thePermanentMemoryCurrent + bytes < thePermanentMemorySize, "Permanent memory overflow");
        if (thePermanentMemoryCurrent + bytes < thePermanentMemorySize) {
            void* result = thePermanentMemory + thePermanentMemoryCurrent;
            thePermanentMemoryCurrent += bytes;
            thePermanentMemoryMutex.unlock();

            memset(result, 0, bytes);

            return result;
        }

        thePermanentMemoryMutex.unlock();

        return nullptr;
    }

    bool Core::InputKeyDown(KeyCode keyCode) {
        return input.keys[keyCode];
    }

    bool Core::InputKeyUp(KeyCode keyCode) {
        return !input.keys[keyCode];
    }

    bool Core::InputKeyJustPressed(KeyCode keyCode) {
        return input.keys[keyCode] && !input.lastKeys[keyCode];
    }

    bool Core::InputKeyJustReleased(KeyCode keyCode) {
        return !input.keys[keyCode] && input.lastKeys[keyCode];
    }

    bool Core::InputMouseButtonDown(MouseButton button) {
        return input.mouseButtons[button];
    }

    bool Core::InputMouseButtonUp(MouseButton button) {
        return !input.mouseButtons[button];
    }

    bool Core::InputMouseButtonJustPressed(MouseButton button) {
        return input.mouseButtons[button] && !input.lastMouseButtons[button];
    }

    bool Core::InputMouseButtonJustReleased(MouseButton button) {
        return !input.mouseButtons[button] && input.lastMouseButtons[button];
    }

    glm::vec2 Core::InputMousePosPixels() {
        return input.mousePosPixels;
    }

    FrameInput & Core::InputGetFrameInput() {
        return input;
    }

    NetClient * Core::GetNetClient() {
        return client;
    }

    SimLogic * Core::GetSimLogic() {
        return simLogic;
    }

    MultiplayerState * Core::GetMPState() {
        return &mpState;
    }

    FixedQueue<NetworkMessage, 1024> & Core::GetGGPOMessages() {
        return mpState.messages;
    }

    FontResource * Core::GetDebugFont() {
        return arialFont;
    }

    void * Core::MemoryAllocateTransient( u64 bytes ) {
        theTransientMemoryMutex.lock();

        AssertMsg(theTransientMemoryCurrent + bytes < theTransientMemorySize, "Transient memory overflow");
        if (theTransientMemoryCurrent + bytes < theTransientMemorySize) {
            void* result = theTransientMemory + theTransientMemoryCurrent;
            theTransientMemoryCurrent += bytes;
            theTransientMemoryMutex.unlock();

            memset(result, 0, bytes);

            return result;
        }

        theTransientMemoryMutex.unlock();

        return nullptr;
    }

    void Core::MemoryMakePermanent(u64 bytes) {
        thePermanentMemory = (u8*)malloc(bytes);
        thePermanentMemorySize = bytes;
    }

    void Core::MemoryClearPermanent() {
        thePermanentMemoryMutex.lock();
        
#if ATTO_DEBUG
        memset(thePermanentMemory, 0, thePermanentMemorySize);
#endif

        thePermanentMemoryCurrent = 0;
        thePermanentMemoryMutex.unlock();
    }

    void Core::MemoryMakeTransient(u64 bytes) {
        theTransientMemory = (u8*)malloc(bytes);
        theTransientMemorySize = bytes;
    }

    void Core::MemoryClearTransient() {
        theTransientMemoryMutex.lock();

#if ATTO_DEBUG
        memset(theTransientMemory, 0, theTransientMemorySize);
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



}


