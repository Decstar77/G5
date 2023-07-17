
#include "atto_core.h"
#include "atto_client.h"

namespace atto {

    f32 Core::GetDeltaTime() const {
        return deltaTime;
    }

    Camera Core::RenderCreateCamera() {
        Camera cam = {};
        cam.zoom = 1.0f;
        cam.width = 320;
        cam.height = 180;
        return cam;
    }

    void Core::RenderSetCamera(Camera* camera) {
        this->camera = camera;
        if (camera != nullptr) {
            f32 hw = camera->width / 2.0f;
            f32 hh = camera->height / 2.0f;
            camera->mainSurfaceWidth = mainSurfaceWidth;
            camera->mainSurfaceHeight = mainSurfaceHeight;
            camera->p = glm::ortho(-hw, hw, -hh, hh, -1.0f, 1.0f);
            camera->v = glm::translate(glm::mat4(1), glm::vec3(-camera->pos, 0));
        }
    }

    void Core::RenderDrawCircle(glm::vec2 pos, f32 radius, glm::vec4 colour /*= glm::vec4(1)*/) {
        if (camera != nullptr) {
            pos = camera->WorldPointToScreen(pos);
        }

        DrawCommand cmd = {};
        cmd.type = DrawCommandType::CIRCLE;
        cmd.color = colour;
        cmd.circle.c = pos;
        cmd.circle.r = radius;
        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawRect(glm::vec2 bl, glm::vec2 tr, glm::vec4 colour /*= glm::vec4(1)*/) {
        if (camera != nullptr) {
            bl = camera->WorldPointToScreen(bl);
            tr = camera->WorldPointToScreen(tr);
        }

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
        if (camera != nullptr) {
            center = camera->WorldPointToScreen(center);
        }

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
        //glm::vec2 direction = glm::normalize(end - start);
        //glm::vec2 perpendicular(direction.y, -direction.x);

        //glm::vec2 points[] = {
        //    start + perpendicular * (thicc / 2.0f),
        //    start - perpendicular * (thicc / 2.0f),
        //    end + perpendicular * (thicc / 2.0f),
        //    end - perpendicular * (thicc / 2.0f)
        //};

        //Geometry::SortPointsIntoClockWiseOrder(points, 4);

        //DrawCommand cmd = {};
        //cmd.type = DRAW_SHAPE_TYPE_LINE;
        //cmd.color = color;
        //cmd.p1 = points[0];
        //cmd.p2 = points[1];
        //cmd.p3 = points[2];
        //cmd.p4 = points[3];


        //drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawSprite(TextureResource* texture, glm::vec2 center, glm::vec2 size, glm::vec4 colour) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::SPRITE;
        cmd.color = colour;
        cmd.sprite.proj = camera != nullptr ? camera->p * camera->v : screenProjection;
        cmd.sprite.textureRes = texture;
        
        glm::vec2 dim = glm::vec2(texture->width, texture->height) * size;
        cmd.rect.bl = -dim / 2.0f;
        cmd.rect.tr = dim / 2.0f;
        cmd.rect.br = glm::vec2(cmd.rect.tr.x, cmd.rect.bl.y);
        cmd.rect.tl = glm::vec2(cmd.rect.bl.x, cmd.rect.tr.y);

        cmd.rect.bl += center;
        cmd.rect.tr += center;
        cmd.rect.br += center;
        cmd.rect.tl += center;

        drawCommands.drawList.Add(cmd);
    }

    void Core::RenderDrawText(FontResource* font, glm::vec2 bl, const char* text, glm::vec4 colour /*= glm::vec4(1)*/) {
        DrawCommand cmd = {};
        cmd.type = DrawCommandType::TEXT;
        cmd.color = colour;
        cmd.text.bl = bl;
        cmd.text.text = text;
        cmd.text.proj = camera != nullptr ? camera->p * camera->v : screenProjection;
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

        Assert(thePermanentMemoryCurrent + bytes < thePermanentMemorySize, "Permanent memory overflow");
        if (thePermanentMemoryCurrent + bytes < thePermanentMemorySize) {
            void* result = thePermanentMemory + thePermanentMemoryCurrent;
            thePermanentMemoryCurrent += bytes;
            thePermanentMemoryMutex.unlock();

            memset(result, 0, bytes);

            return result;
        }

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

    FrameInput& Core::InputGetFrameInput() {
        return input;
    }

    void* Core::MemoryAllocateTransient(u64 bytes) {
        theTransientMemoryMutex.lock();

        Assert(theTransientMemoryCurrent + bytes < theTransientMemorySize, "Transient memory overflow");
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

    glm::vec2 Camera::ScreenPointToWorld(glm::vec2 screen) {
        glm::vec4 viewport = glm::vec4(0, 0, mainSurfaceWidth, mainSurfaceHeight);
        glm::vec3 win(screen.x, screen.y, 0);
        glm::vec3 world = glm::unProject(win, v, p, viewport);

        return glm::vec2(world.x, world.y);

    }

    glm::vec2 Camera::WorldPointToScreen(glm::vec2 world){
        glm::vec4 viewport = glm::vec4(0, 0, mainSurfaceWidth, mainSurfaceHeight);
        glm::vec3 win = glm::project(glm::vec3(world.x, world.y, 0), v, p, viewport);

        return glm::vec2(win.x, win.y);
    }
}


