#pragma once

#include "../shared/atto_core.h"

struct FONScontext;
typedef FONScontext * FontContext;

namespace FMOD {
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
}

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
    };

    struct Win32AudioResource : public AudioResource {
        FMOD::Sound * sound2D;
        FMOD::Sound * sound3D;
    };

    struct Win32StaticMeshResource : public StaticMeshResource {
        u32 vao;
        u32 vbo;
        u32 ibo;
    };

    struct ResourceRegistry {
        FixedList<Win32TextureResource, 1024>       textures;
        FixedList<Win32AudioResource, 1024>         audios;
        FixedList<SpriteResource, 1024>             sprites;
        FontContext                                 fontContext;
    };

    class WindowsCore : public Core {
    public:
        void Run(int argc, char** argv) override;
        
        virtual f64                     GetTheCurrentTime() const override;

        virtual TextureResource *       ResourceGetAndLoadTexture( const char * name, bool genMips, bool genAnti ) override;
        virtual TextureResource *       ResourceRegisterTexture( TextureResource * src ) override;
        
        virtual SpriteResource *        ResourceGetAndCreateSprite( const char * spriteName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) override;
        virtual SpriteResource *        ResourceGetAndLoadSprite( const char * spriteName ) override;
        virtual SpriteResource *        ResourceGetLoadedSprite( i64 spriteId );
        
        virtual AudioResource *         ResourceGetAndCreateAudio( const char * name, bool is2D, bool is3D, f32 minDist, f32 maxDist ) override;
        
        virtual FontHandle              ResourceGetFont( const char * name ) override;
        
        virtual void                    ResourceReadEntireTextFile( const char * path, char * data, i32 maxLen ) override;
        virtual void                    ResourceWriteEntireTextFile( const char * path, const char * data ) override;
        virtual void                    ResourceReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) override;
        virtual void                    ResourceWriteEntireBinaryFile( const char * path, const char * data, i32 size ) override;
        virtual i64                     ResourceGetFileSize( const char * path ) override;

        virtual AudioSpeaker            AudioPlay( AudioResource * audioResource, glm::vec2 * pos ) override;
        virtual void                    AudioSetListener( glm::vec2 pos ) override;

        virtual float                   FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds ) override;
        virtual void                    RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) override;
        
        virtual void                    InputDisableMouse() override;
        virtual void                    InputEnableMouse() override;
        virtual bool                    InputIsMouseDisabled() override;

        virtual void                    WindowClose() override;
        virtual void                    WindowSetTitle(const char* title) override;
        virtual void                    WindowSetVSync( bool value ) override;
        virtual bool                    WindowGetVSync() override;
        virtual bool                    WindowOpenNativeFileDialog( const char * basePath, const char * filter, LargeString & res ) override;
        virtual bool                    WindowOpenNativeFolderDialog( const char * basePath, LargeString & res ) override;

        void                            GLResetSurface(f32 w, f32 h);

        static void                     WinBoyoWriteTextFile( const char * path, const char * text );
        static void                     WinBoyoReadTextFile( const char * path, char * text, i32 maxLen );
        static void                     WinBoyoWriteBinaryFile( const char * path, const char * data, i64 size );
        static void                     WinBoyoReadBinaryFile( const char * path, char * data, i64 size );

    #if ATTO_EDITOR
        virtual void                    EditorOnly_SaveLoadedResourcesToBinary() override;
    #endif

    public:
        ResourceRegistry            resources = {};
        BinaryBlob                  textureBlob = {};

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

        FMOD::ChannelGroup *            fmodMasterGroup;
        FMOD::System *                  fmodSystem;
        FixedList<AudioSpeaker, 32>     alSpeakers;

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

        bool            AudioInitialize();
        void            AudioUpdate();
        void            AudioShudown();
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

