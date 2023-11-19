#include "atto_core_windows.h"
#include "../shared/atto_colors.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <glad/glad.h>

#define FONTSTASH_IMPLEMENTATION	// Expands implementation
#include <fontstash.h>

namespace atto {

    static int glfons__renderCreate( void * userPtr, int width, int height ) {
        WindowsCore * core = (WindowsCore *)userPtr;
        core->GLFontsRenderCreate( width, height );
        return 1;
    }

    static int glfons__renderResize( void * userPtr, int width, int height ) {
        return glfons__renderCreate( userPtr, width, height );
    }

    static void glfons__renderUpdate( void * userPtr, int * rect, const unsigned char * data ) {
        WindowsCore * core = (WindowsCore *)userPtr;
        core->GLFontsRenderUpdate( rect, data );
    }

    static void glfons__renderDraw( void * userPtr, const float * verts, const float * tcoords, const unsigned int * colors, int nverts ) {
        WindowsCore * core = (WindowsCore *)userPtr;
        core->GLFontsRenderDraw( verts, tcoords, colors, nverts );
    }

    static void glfons__renderDelete( void * userPtr ) {
        WindowsCore * core = (WindowsCore *)userPtr;
        core->GLFontsRenderDelete();
    }

    void WindowsCore::GLInitializeTextRendering() {
        i32 textureWidth = 512;
        i32 textureHeight = 512;
        i32 textureFlags = FONS_ZERO_TOPLEFT;

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

        resources.fontContext = fonsCreateInternal( &params );
        arialFontHandle = fonsAddFont( resources.fontContext, "default", "res/fonts/arial.ttf" );
        kenFontHandle = fonsAddFont( resources.fontContext, "ken", "res/fonts/kenvector_future.ttf" );

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

        VertexLayoutFont text = {};
        fontProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        fontVertexBuffer = GLCreateVertexBuffer( &text, FONS_VERTEX_COUNT, nullptr, true );
    }

    void WindowsCore::RenderDrawCommandText( DrawCommand & cmd ) {
        FontContext fs = resources.fontContext;
        GLEnableAlphaBlending();
        GLShaderProgramBind( fontProgram );
        GLShaderProgramSetSampler( "TextureSampler", 0 );
        GLShaderProgramSetMat4( "p", screenProjection );

        u32 color = Colors::VecToU32( cmd.color );
        fonsSetFont( fs, cmd.text.font.idx );
        fonsSetSize( fs, cmd.text.fontSize );
        fonsSetColor( fs, color );
        fonsDrawText( fs, cmd.text.bl.x, cmd.text.bl.x, cmd.text.text.GetCStr(), NULL);
    }

    FontHandle WindowsCore::ResourceGetFont( const char * name ) {
        i32 fh = fonsGetFontByName( resources.fontContext, name );
        if( fh == FONS_INVALID ) {
            return FontHandle::INVALID;
        }

        FontHandle fontHandle = {};
        fontHandle.idx = fh;
        fontHandle.gen = 0;
        return fontHandle;
    }

    void WindowsCore::GLFontsRenderCreate( i32 width, i32 height ) {
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

    void WindowsCore::GLFontsRenderUpdate( i32 * rect, const byte * data ) {
        i32 w = rect[ 2 ] - rect[ 0 ];
        i32 h = rect[ 3 ] - rect[ 1 ];

        if( fontTextureHandle == 0 ) return;

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

    void WindowsCore::GLFontsRenderDraw( const f32 * verts, const f32 * tcoords, const u32 * colors, i32 nverts ) {
        const i32 size = sizeof( VertexLayoutFont::FontVertex ) * nverts;
        VertexLayoutFont::FontVertex * vertices = (VertexLayoutFont::FontVertex *)MemoryAllocateTransient( size );
        for( i32 i = 0; i < nverts; i++ ) {
            vertices[ i ].position.x = verts[ 2 * i ];
            vertices[ i ].position.y = verts[ 2 * i + 1 ];

            vertices[ i ].uv.x = tcoords[ 2 * i ];
            vertices[ i ].uv.y = tcoords[ 2 * i + 1 ];

            vertices[ i ].colour = colors[ i ];
        }

        GLShaderProgramBind( fontProgram );
        GLShaderProgramSetMat4( "p", screenProjection );
        GLShaderProgramSetTexture( "TextureSampler", fontTextureHandle, 0 );

        glBindVertexArray( fontVertexBuffer.vao );
        GLVertexBufferUpdate( fontVertexBuffer, 0, size, vertices );
        glDrawArrays( GL_TRIANGLES, 0, nverts );
        glBindVertexArray( 0 );
    }

    void WindowsCore::GLFontsRenderDelete() {

    }

    void VertexLayoutFont::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, false, stride, 0 );

        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, true, stride, (void *)( 3 * sizeof( f32 ) ) );

        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, false, stride, (void *)( 4 * sizeof( f32 ) ) );
    }

    i32 VertexLayoutFont::SizeBytes() {
        return (i32)sizeof( FontVertex );
    }

    i32 VertexLayoutFont::StrideBytes() {
        return (i32)sizeof( FontVertex );
    }
}