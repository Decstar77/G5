#pragma once

#include "../shared/atto_core.h"

struct ALCdevice;
struct ALCcontext;

struct FONScontext;
typedef FONScontext * FontContext;

namespace atto {
    struct ShaderUniform {
        SmallString name;
        i32         location;
    };

    struct ShaderProgram {
        u32                             programHandle;
        FixedList<ShaderUniform, 16>    uniforms;
    };

    struct VertexBuffer {
        u32 vao;
        u32 vbo;
        i32 size;
        i32 stride;
    };

    class VertexLayout {
    public:
        virtual void Layout() = 0;
        virtual i32  SizeBytes() = 0;
        virtual i32 StrideBytes() = 0;
    };

    class VertexLayoutShape : public VertexLayout {
    public:
        struct ShapeVertex {
            glm::vec2 position;
        };
        
        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    class VertexLayoutSprite : public VertexLayout {
    public:
        struct SpriteVertex {
            glm::vec2 position;
            glm::vec2 uv;
        };

        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    class VertexLayoutText : public VertexLayout {
    public:
        struct TextVertex {
            glm::vec2 position;
            glm::vec2 uv;
        };

        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    struct Win32TextureResource : public TextureResource {
        u32 handle;
    };

    struct Win32AudioResource : public AudioResource {
        u32 handle;
    };

    struct ResourceRegistry {
        FixedList<Win32TextureResource, 1024>       textures;
        FixedList<Win32AudioResource, 1024>         audios;
        FontContext                                 fontContext;
    };

    class WindowsCore : public Core {
    public:
        void Run(int argc, char** argv) override;
        
        virtual TextureResource*    ResourceGetAndLoadTexture(const char* name) override;
        virtual AudioResource*      ResourceGetAndLoadAudio(const char* name) override;
        virtual FontHandle          ResourceGetFont( const char * name ) override;

        virtual AudioSpeaker        AudioPlay(AudioResource* audioResource, f32 volume = 1.0f, bool looping = false) override;

        void                        RenderSetCamera( f32 width, f32 height ) override;
        void                        RenderSubmit() override;

        void WindowClose() override;
        void WindowSetTitle(const char* title) override;

        void                        GLResetSurface(f32 w, f32 h);

    protected:
        ResourceRegistry    resources = {};

        ShaderProgram * boundProgram;

        ShaderProgram               shapeProgram;
        VertexBuffer                shapeVertexBuffer;
        ShaderProgram               spriteProgram;
        VertexBuffer                spriteVertexBuffer;
        ShaderProgram               textProgram;
        VertexBuffer                textVertexBuffer;
        
        ALCdevice*                  alDevice = nullptr;
        ALCcontext*                 alContext = nullptr;
        FixedList<AudioSpeaker, 32> alSpeakers;

        i32 arialFontHandle;
        i32 kenFontHandle;

        u64             OsGetFileLastWriteTime(const char* fileName) override;
        void            OsLogMessage(const char* message, u8 colour) override;
        void            OsErrorBox(const char* msg) override;
        void            OsParseStartArgs(int argc, char** argv);

        bool            ALInitialize();
        void            ALShudown();
        void            ALCheckErrors();
        u32             ALGetFormat(u32 numChannels, u32 bitDepth);
        u32             ALCreateAudioBuffer(i32 sizeBytes, byte* data, i32 channels, i32 bitDepth, i32 sampleRate);
        bool            ALLoadOGG(const char* file, Win32AudioResource& audioBuffer);
        bool            ALLoadWAV(const char* file, Win32AudioResource& audioBuffer);

        void            GLShaderProgramBind(ShaderProgram& program);
        i32             GLShaderProgramGetUniformLocation(ShaderProgram& program, const char* name);
        void            GLShaderProgramSetInt(const char* name, i32 value);
        void            GLShaderProgramSetSampler(const char* name, i32 value);
        void            GLShaderProgramSetTexture(i32 location, u32 textureHandle);
        void            GLShaderProgramSetFloat(const char* name, f32 value);
        void            GLShaderProgramSetVec2(const char* name, glm::vec2 value);
        void            GLShaderProgramSetVec3(const char* name, glm::vec3 value);
        void            GLShaderProgramSetVec4(const char* name, glm::vec4 value);
        void            GLShaderProgramSetMat3(const char* name, glm::mat3 value);
        void            GLShaderProgramSetMat4(const char* name, glm::mat4 value);

        void            GLEnableAlphaBlending();
        void            GLEnablePreMultipliedAlphaBlending();

        bool            GLCheckShaderCompilationErrors(u32 shader);
        bool            GLCheckShaderLinkErrors(u32 program);
        ShaderProgram   GLCreateShaderProgram(const char* vertexSource, const char* fragmentSource);
        VertexBuffer    GLCreateVertexBuffer( VertexLayout * layout, i32 vertexCount, const void* srcData, bool dyanmic);
        void            GLVertexBufferUpdate(VertexBuffer vertexBuffer, i32 offset, i32 size, const void* data);

        void            GLInitializeShapeRendering();
        void            GLInitializeSpriteRendering();
        void            GLInitializeTextRendering();

    };
}

