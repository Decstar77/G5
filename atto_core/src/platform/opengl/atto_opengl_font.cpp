#include "atto_opengl.h"

#if ATTO_OPENGL

#include "../atto_core_windows.h"
#include "../../shared/atto_colors.h"

// @NOTE: I can't believe it need this...
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <glad/glad.h>

#define FONTSTASH_IMPLEMENTATION	// Expands implementation
#include <fontstash.h>

namespace atto {

    static int glfons__renderCreate( void * userPtr, int width, int height ) {
        OpenglState * glState = (OpenglState *)userPtr;
        glState->GLFontsRenderCreate( width, height );
        return 1;
    }

    static int glfons__renderResize( void * userPtr, int width, int height ) {
        return glfons__renderCreate( userPtr, width, height );
    }

    static void glfons__renderUpdate( void * userPtr, int * rect, const unsigned char * data ) {
        OpenglState * glState = (OpenglState *)userPtr;
        glState->GLFontsRenderUpdate( rect, data );
    }

    static void glfons__renderDraw( void * userPtr, const float * verts, const float * tcoords, const unsigned int * colors, int nverts ) {
        OpenglState * glState = (OpenglState *)userPtr;
        glState->GLFontsRenderDraw( verts, tcoords, colors, nverts );
    }

    static void glfons__renderDelete( void * userPtr ) {
        OpenglState * glState = (OpenglState *)userPtr;
        glState->GLFontsRenderDelete();
    }
    
    FontHandle OpenglState::ResourceGetFont( const char * name ) {
        i32 fh = fonsGetFontByName( core->resources.fontContext, name );
        if( fh == FONS_INVALID ) {
            return FontHandle::INVALID;
        }

        FontHandle fontHandle = {};
        fontHandle.idx = fh;
        fontHandle.gen = 0;
        return fontHandle;
    }

    float OpenglState::FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA, TextAlignment_V vA ) {
        FontContext fs = core->resources.fontContext;
        float fbounds[ 4 ] = {};
        fonsSetFont( fs, font.idx );
        fonsSetSize( fs, fontSize );
        fonsSetAlign( fs, (i32)hA | (i32)vA ); //FONS_ALIGN_CENTER | FONS_ALIGN_BASELINE
        float r = fonsTextBounds( fs, pos.x, pos.y, text, NULL, fbounds );
        bounds.min.x = fbounds[ 0 ];
        bounds.min.y = fbounds[ 1 ];
        bounds.max.x = fbounds[ 2 ];
        bounds.max.y = fbounds[ 3 ];
        return r;
    }

    void OpenglState::RenderDrawCommandText( DrawCommand & cmd ) {
        FontContext fs = core->resources.fontContext;

        GLEnableAlphaBlending();
        GLShaderProgramBind( fontProgram );
        GLShaderProgramSetSampler( "TextureSampler", 0 );
        GLShaderProgramSetTexture( "TextureSampler", fontTextureHandle, 0 );
        GLShaderProgramSetMat4( "p", cmd.proj );

        u32 color = Colors::VecToU32( cmd.color );
        fonsSetFont( fs, cmd.text.font.idx );
        fonsSetSize( fs, cmd.text.fontSize );
        fonsSetAlign( fs, cmd.text.align );
        fonsSetColor( fs, color );
        fonsDrawText( fs, cmd.text.bl.x, cmd.text.bl.y, cmd.text.text.GetCStr(), NULL );
    }

    void OpenglState::GLInitializeTextRendering() {
        i32 textureWidth = 512;
        i32 textureHeight = 512;
        i32 textureFlags = FONS_ZERO_BOTTOMLEFT;

        FONSparams params = {};
        params.width = textureWidth;
        params.height = textureHeight;
        params.flags = (unsigned char)textureFlags;
        params.renderCreate = glfons__renderCreate;
        params.renderResize = glfons__renderResize;
        params.renderUpdate = glfons__renderUpdate;
        params.renderDraw   = glfons__renderDraw;
        params.renderDelete = glfons__renderDelete;
        params.userPtr = this;

        core->resources.fontContext = fonsCreateInternal( &params );
        arialFontHandle = fonsAddFont( core->resources.fontContext, "default", "res/game/fonts/arial.ttf" );
        kenFontHandle = fonsAddFont( core->resources.fontContext, "ken", "res/game/fonts/kenvector_future.ttf" );

        const char * vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 a_position;
            layout (location = 1) in vec4 a_color;
            layout (location = 2) in vec2 a_texCoords0;

            uniform mat4 p;

            out vec4 v_color;
            out vec2 v_texCoords;

            void main()
            {
                v_color = a_color;
                v_texCoords = a_texCoords0;
                gl_Position = p * vec4(a_position, 1.0);
            }
        )";

        const char * fragmentShaderSource = R"(
            #version 330 core
            uniform sampler2D TextureSampler;

            in vec4 v_color;
            in vec2 v_texCoords;

            out vec4 FragColor;
            void main()
            {
                //FragColor = vec4(v_texCoords, 0.0, 1.0);
                FragColor = v_color * texture2D(TextureSampler, v_texCoords);
            }
        )";

        GLVertexLayoutFont text = {};
        fontProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        fontVertexBuffer = GLCreateVertexBuffer( &text, FONS_VERTEX_COUNT, nullptr, true );
    }

    void OpenglState::GLFontsRenderCreate( i32 width, i32 height ) {
        if ( fontTextureHandle != 0 ) {
            glDeleteTextures( 1, &fontTextureHandle );
            fontTextureHandle = 0;
        }

        fontTextureWidth = width;
        fontTextureHeight = height;

        glGenTextures( 1, &fontTextureHandle );
        glBindTexture( GL_TEXTURE_2D, fontTextureHandle );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glGenerateMipmap( GL_TEXTURE_2D );

        static GLint swizzleRgbaParams[ 4 ] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
        glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleRgbaParams );

        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    void OpenglState::GLFontsRenderUpdate( i32 * rect, const byte * data ) {
        i32 w = rect[ 2 ] - rect[ 0 ];
        i32 h = rect[ 3 ] - rect[ 1 ];

        if( fontTextureHandle == 0 ) { 
            return;
        }

        // Push old values
        GLint alignment, rowLength, skipPixels, skipRows;
        glGetIntegerv( GL_UNPACK_ALIGNMENT, &alignment );
        glGetIntegerv( GL_UNPACK_ROW_LENGTH, &rowLength );
        glGetIntegerv( GL_UNPACK_SKIP_PIXELS, &skipPixels );
        glGetIntegerv( GL_UNPACK_SKIP_ROWS, &skipRows );

        glBindTexture( GL_TEXTURE_2D, fontTextureHandle );

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glPixelStorei( GL_UNPACK_ROW_LENGTH, fontTextureWidth );
        glPixelStorei( GL_UNPACK_SKIP_PIXELS, rect[ 0 ] );
        glPixelStorei( GL_UNPACK_SKIP_ROWS, rect[ 1 ] );

        glTexSubImage2D( GL_TEXTURE_2D, 0, rect[ 0 ], rect[ 1 ], w, h, GL_RED, GL_UNSIGNED_BYTE, data );

        // Pop old values
        glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );
        glPixelStorei( GL_UNPACK_ROW_LENGTH, rowLength );
        glPixelStorei( GL_UNPACK_SKIP_PIXELS, skipPixels );
        glPixelStorei( GL_UNPACK_SKIP_ROWS, skipRows );
    }

    void OpenglState::GLFontsRenderDraw( const f32 * verts, const f32 * tcoords, const u32 * colors, i32 nverts ) {
        const i32 size = sizeof( GLVertexLayoutFont::FontVertex ) * nverts;
        GLVertexLayoutFont::FontVertex * vertices = (GLVertexLayoutFont::FontVertex *)core->MemoryAllocateTransient( size );
        for( i32 i = 0; i < nverts; i++ ) {
            vertices[ i ].position.x = verts[ 2 * i ];
            vertices[ i ].position.y = verts[ 2 * i + 1 ];

            vertices[ i ].uv.x = tcoords[ 2 * i ];
            vertices[ i ].uv.y = tcoords[ 2 * i + 1 ];

            vertices[ i ].colour = colors[ i ];
        }

        glBindVertexArray( fontVertexBuffer.vao );
        GLVertexBufferUpdate( fontVertexBuffer, 0, size, vertices );
        glDrawArrays( GL_TRIANGLES, 0, nverts );
        glBindVertexArray( 0 );
    }

    void OpenglState::GLFontsRenderDelete() {

    }

    void GLVertexLayoutFont::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, false, stride, 0 );

        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, true, stride, (void *)( 3 * sizeof( f32 ) ) );

        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, false, stride, (void *)( 4 * sizeof( f32 ) ) );
    }

    i32 GLVertexLayoutFont::SizeBytes() {
        return (i32)sizeof( FontVertex );
    }

    i32 GLVertexLayoutFont::StrideBytes() {
        return (i32)sizeof( FontVertex );
    }
}

#endif