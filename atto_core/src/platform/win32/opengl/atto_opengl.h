#pragma once

#include "../../../game/atto_core.h"

#if ATTO_OPENGL

namespace atto {
    class WindowsCore;
    class DrawContext;
    struct DrawCommand;

    struct GLShaderUniform {
        SmallString name;
        i32         location;
    };

    struct GLShaderProgram {
        u32                               programHandle;
        FixedList<GLShaderUniform, 16>    uniforms;
    };

    struct GLVertexBuffer {
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

    class GLVertexLayoutShape : public VertexLayout {
    public:
        struct ShapeVertex {
            glm::vec2 position;
        };
        
        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    class GLVertexLayoutSprite : public VertexLayout {
    public:
        struct SpriteVertex {
            glm::vec2 position;
            glm::vec2 uv;
        };

        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    class GLVertexLayoutFont : public VertexLayout {
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

    class GLVertexLayoutStaticModel : public VertexLayout {
    public:
        void Layout() override;
        i32 SizeBytes() override;
        i32 StrideBytes() override;
    };

    struct GLStaticMeshResource : public StaticMeshResource {
        u32 vao;
        u32 vbo;
        u32 ibo;
    };

    class OpenglState {
    public:
        void                Initialize( WindowsCore * core );
        void                RenderSubmit( DrawContext * dcxt, bool clearBackBuffers );

        void                GLCheckCapablities();
        void                GLResetSurface( f32 w, f32 h, f32 cameraWidth, f32 cameraHeight );
        void                GLSetCamera( f32 width, f32 height );

        void                GLShaderProgramBind( GLShaderProgram & program );
        i32                 GLShaderProgramGetUniformLocation( GLShaderProgram & program, const char * name );
        void                GLShaderProgramSetInt( const char * name, i32 value );
        void                GLShaderProgramSetSampler( const char * name, i32 value );
        void                GLShaderProgramSetTexture( i32 location, u32 textureHandle );
        void                GLShaderProgramSetTexture( const char * name, u32 textureHandle, i32 slot );
        void                GLShaderProgramSetFloat( const char * name, f32 value );
        void                GLShaderProgramSetVec2( const char * name, glm::vec2 value );
        void                GLShaderProgramSetVec3( const char * name, glm::vec3 value );
        void                GLShaderProgramSetVec4( const char * name, glm::vec4 value );
        void                GLShaderProgramSetMat3( const char * name, glm::mat3 value );
        void                GLShaderProgramSetMat4( const char * name, glm::mat4 value );

        void                GLEnableAlphaBlending();
        void                GLEnablePreMultipliedAlphaBlending();

        bool                GLCheckShaderCompilationErrors( u32 shader );
        bool                GLCheckShaderLinkErrors( u32 program );
        GLShaderProgram     GLCreateShaderProgram( const char * vertexSource, const char * fragmentSource );
        GLVertexBuffer      GLCreateVertexBuffer( VertexLayout * layout, i32 vertexCount, const void * srcData, bool dyanmic );
        void                GLVertexBufferUpdate( GLVertexBuffer vertexBuffer, i32 offset, i32 size, const void * data );
        void                GLVertexBufferUpdate( u32 vertexBuffer, i32 offset, i32 size, const void * data );

        void                GLInitializeShapeRendering();
        void                GLInitializeSpriteRendering();
        void                GLInitializeTextRendering();
        void                GLInitializeUnlitModelRendering();

        FontHandle          ResourceGetFont( const char * name );
        float               FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA, TextAlignment_V vA );
        void                RenderDrawCommandText( DrawCommand & cmd );
        void                GLFontsRenderCreate( i32 width, i32 height );
        void                GLFontsRenderUpdate( i32 * rect, const byte * data );
        void                GLFontsRenderDraw( const f32 * verts, const f32 * tcoords, const u32 * colors, i32 nverts );
        void                GLFontsRenderDelete();

        FixedList<TextureResource, 1024>    textures = {};
        WindowsCore *                       core = nullptr;

        GLShaderProgram *                   boundProgram = nullptr;
        GLShaderProgram                     shapeProgram = {};
        GLVertexBuffer                      shapeVertexBuffer = {};
        GLShaderProgram                     spriteProgram = {};
        GLVertexBuffer                      spriteVertexBuffer = {};
        GLVertexBuffer                      spritePackedVertexBuffer = {};

        GLShaderProgram                     staticMeshUnlitProgram = {};
        GLStaticMeshResource *              staticMeshTriangle= nullptr;
        GLStaticMeshResource *              staticMeshPlane = nullptr;
        GLStaticMeshResource *              staticMeshBox = nullptr;
        GLStaticMeshResource *              staticMeshSphere = nullptr;
        GLStaticMeshResource *              staticMeshCylinder = nullptr;

        GLShaderProgram                     fontProgram = {};
        GLVertexBuffer                      fontVertexBuffer = {};
        i32                                 fontTextureWidth = 0;
        i32                                 fontTextureHeight = 0;
        u32                                 fontTextureHandle = 0;
        i32                                 arialFontHandle = 0;
        i32                                 kenFontHandle = 0;

        FixedList<glm::vec2, 96>            shapeCirleBase = {};
        FixedList<glm::vec2, 96>            shapeCirle = {};
    };
    
}

#endif 