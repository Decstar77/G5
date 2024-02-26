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

    class VertexLayoutFont : public VertexLayout {
    public:
        struct FontVertex {
            glm::vec3   position;
            u32         colour;
            glm::vec2   uv;
        };

        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    class VertexLayoutStaticModel : public VertexLayout {
        
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

    struct Win32StaticMeshResource : public StaticMeshResource {
        u32 vao;
        u32 vbo;
        u32 ibo;
    };

    struct ResourceRegistry {
        FixedList<Win32TextureResource, 1024>       textures;
        FixedList<Win32AudioResource, 1024>         audios;
        FixedList<Win32StaticMeshResource, 1024>    meshes;
        FixedList<SpriteResource, 1024>             sprites;
        FontContext                                 fontContext;
    };

    class WindowsCore : public Core {
    public:
        void Run(int argc, char** argv) override;
        
        virtual TextureResource *       ResourceGetAndLoadTexture( const char * name, bool genMips, bool genAnti ) override;
        virtual SpriteResource *        ResourceGetAndCreateSprite( const char * spriteName, const char * textureName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) override;
        virtual AudioResource *         ResourceGetAndLoadAudio( const char * name ) override;
        virtual StaticMeshResource *    ResourceGetAndLoadMesh( const char * name ) override;
        virtual FontHandle              ResourceGetFont( const char * name ) override;
        virtual void                    ResourceReadEntireFile( const char * path, char * data, i32 maxLen ) override;
        virtual void                    ResourceWriteEntireFile( const char * path, const char * data ) override;

        StaticMeshResource *            ResourceMeshCreate( const char * name, StaticMeshData & data );
        StaticMeshResource *            ResourceMeshCreate( const char * name, i32 vertexCount );

        virtual AudioSpeaker            AudioPlay( AudioResource * audioResource, f32 volume = 1.0f, bool looping = false ) override;

        virtual float                   FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds ) override;
        virtual void                    RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) override;
        
        virtual void                    InputDisableMouse() override;
        virtual void                    InputEnableMouse() override;
        virtual bool                    InputIsMouseDisabled() override;

        virtual void                    WindowClose() override;
        virtual void                    WindowSetTitle(const char* title) override;
        virtual void                    WindowSetVSync( bool value ) override;
        virtual bool                    WindowGetVSync() override;

        void                        GLResetSurface(f32 w, f32 h);

        static void                 WinBoyoWriteTextFile( const char * path, const char * text );
        static void                 WinBoyoReadTextFile( const char * path, char * text, i32 maxLen );

    public:
        ResourceRegistry            resources = {};

        ShaderProgram *             boundProgram;

        ShaderProgram               shapeProgram;
        VertexBuffer                shapeVertexBuffer;
        ShaderProgram               spriteProgram;
        VertexBuffer                spriteVertexBuffer;
        
        ShaderProgram                     staticMeshUnlitProgram;
        Win32StaticMeshResource *         staticMeshTriangle;
        Win32StaticMeshResource *         staticMeshPlane;
        Win32StaticMeshResource *         staticMeshBox;
        Win32StaticMeshResource *         staticMeshSphere;
        Win32StaticMeshResource *         staticMeshCylinder;

        ALCdevice*                  alDevice = nullptr;
        ALCcontext*                 alContext = nullptr;
        FixedList<AudioSpeaker, 32> alSpeakers;

        ShaderProgram               fontProgram;
        VertexBuffer                fontVertexBuffer;
        i32                         fontTextureWidth;
        i32                         fontTextureHeight;
        u32                         fontTextureHandle = 0;
        i32                         arialFontHandle;
        i32                         kenFontHandle;

        void            RenderDrawCommandText( DrawCommand & cmd );
        void            RenderSetCamera( f32 width, f32 height );

        u64             OsGetFileLastWriteTime( const char * fileName ) override;
        void            OsLogMessage( const char * message, u8 colour ) override;
        void            OsErrorBox( const char * msg ) override;
        void            OsParseStartArgs( int argc, char ** argv );

        bool            ALInitialize();
        void            ALShudown();
        void            ALCheckErrors();
        u32             ALGetFormat( u32 numChannels, u32 bitDepth );
        u32             ALCreateAudioBuffer( i32 sizeBytes, byte * data, i32 channels, i32 bitDepth, i32 sampleRate );
        bool            ALLoadOGG( const char * file, Win32AudioResource & audioBuffer );
        bool            ALLoadWAV( const char * file, Win32AudioResource & audioBuffer );

        void            GLCheckCapablities();
        void            GLShaderProgramBind( ShaderProgram & program );
        i32             GLShaderProgramGetUniformLocation( ShaderProgram & program, const char * name );
        void            GLShaderProgramSetInt( const char * name, i32 value );
        void            GLShaderProgramSetSampler( const char * name, i32 value );
        void            GLShaderProgramSetTexture( i32 location, u32 textureHandle );
        void            GLShaderProgramSetTexture( const char * name, u32 textureHandle, i32 slot );
        void            GLShaderProgramSetFloat( const char * name, f32 value );
        void            GLShaderProgramSetVec2( const char * name, glm::vec2 value );
        void            GLShaderProgramSetVec3( const char * name, glm::vec3 value );
        void            GLShaderProgramSetVec4( const char * name, glm::vec4 value );
        void            GLShaderProgramSetMat3( const char * name, glm::mat3 value );
        void            GLShaderProgramSetMat4( const char * name, glm::mat4 value );

        void            GLEnableAlphaBlending();
        void            GLEnablePreMultipliedAlphaBlending();

        bool            GLCheckShaderCompilationErrors( u32 shader );
        bool            GLCheckShaderLinkErrors( u32 program );
        ShaderProgram   GLCreateShaderProgram( const char * vertexSource, const char * fragmentSource );
        VertexBuffer    GLCreateVertexBuffer( VertexLayout * layout, i32 vertexCount, const void * srcData, bool dyanmic );
        void            GLVertexBufferUpdate( VertexBuffer vertexBuffer, i32 offset, i32 size, const void * data );
        void            GetVertexBufferUpdate( u32 vbo, i32 offset, i32 size, const void * data );
        
        void            GLInitializeShapeRendering();
        void            GLInitializeSpriteRendering();
        void            GLInitializeTextRendering();
        void            GLInitializeUnlitModelRendering();

        void            GLFontsRenderCreate( i32 width, i32 height );
        void            GLFontsRenderUpdate( i32 * rect, const byte * data );
        void            GLFontsRenderDraw( const f32 * verts, const f32 * tcoords, const u32 * colors, i32 nverts );
        void            GLFontsRenderDelete();

    };
}

