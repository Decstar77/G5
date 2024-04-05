#include "atto_opengl.h"

#if ATTO_OPENGL

#include "../atto_core_windows.h"
#include "../../content/atto_content.h"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace atto {
    void GLVertexLayoutShape::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, false, stride, 0 );
    }

    i32 GLVertexLayoutShape::SizeBytes() {
        return (i32)sizeof( ShapeVertex );
    }

    i32 GLVertexLayoutShape::StrideBytes() {
        return (i32)sizeof( ShapeVertex );
    }

    void GLVertexLayoutSprite::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glEnableVertexAttribArray( 1 );
        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, false, stride, 0 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, false, stride, (void *)( 2 * sizeof( f32 ) ) );
    }

    i32 GLVertexLayoutSprite::SizeBytes() {
        return (i32)sizeof( SpriteVertex );
    }

    i32 GLVertexLayoutSprite::StrideBytes() {
        return (i32)sizeof( SpriteVertex );
    }

    void GLVertexLayoutStaticModel::Layout() {
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)0 );

        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, normal ) );

        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, uv ) );
    }

    i32 GLVertexLayoutStaticModel::SizeBytes() {
        return (i32)sizeof( StaticMeshVertex );
    }

    i32 GLVertexLayoutStaticModel::StrideBytes() {
        return (i32)sizeof( StaticMeshVertex );
    }

    void OpenglState::GLCheckCapablities() {
    }

    void OpenglState::GLShaderProgramBind( GLShaderProgram & program ) {
        AssertMsg( program.programHandle != 0, "Shader program not created" );
        glUseProgram( program.programHandle );
        boundProgram = &program;
    }

    i32 OpenglState::GLShaderProgramGetUniformLocation( GLShaderProgram & program, const char * name ) {
        if( program.programHandle == 0 ) {
            core->LogOutput( LogLevel::ERR, "Shader program in not valid" );
            return -1;
        }

        const u32 uniformCount = program.uniforms.GetCount();
        for( u32 uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++ ) {
            GLShaderUniform & uniform = program.uniforms[ uniformIndex ];
            if( uniform.name == name ) {
                return uniform.location;
            }
        }

        i32 location = glGetUniformLocation( program.programHandle, name );
        if( location >= 0 ) {
            GLShaderUniform newUniform = {};
            newUniform.location = location;
            newUniform.name = name;

            program.uniforms.Add( newUniform );
        }
        else {
            core->LogOutput( LogLevel::ERR, "Could not find uniform value %s", name );
        }

        return location;
    }

    void OpenglState::GLShaderProgramSetInt( const char * name, i32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1i( location, value );
        }
    }

    void OpenglState::GLShaderProgramSetSampler( const char * name, i32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1i( location, value );
        }
    }

    void OpenglState::GLShaderProgramSetTexture( i32 location, u32 textureHandle ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        glBindTextureUnit( 0, textureHandle );
    }

    void OpenglState::GLShaderProgramSetTexture( const char * name, u32 textureHandle, i32 slot ) {
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        glActiveTexture( GL_TEXTURE0 + slot );
        glBindTexture( GL_TEXTURE_2D, textureHandle );
        glUniform1i( location, slot );
    }

    void OpenglState::GLShaderProgramSetFloat( const char * name, f32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1f( location, value );
        }
    }

    void OpenglState::GLShaderProgramSetVec2( const char * name, glm::vec2 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform2fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void OpenglState::GLShaderProgramSetVec3( const char * name, glm::vec3 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform3fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void OpenglState::GLShaderProgramSetVec4( const char * name, glm::vec4 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform4fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void OpenglState::GLShaderProgramSetMat3( const char * name, glm::mat3 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniformMatrix3fv( location, 1, GL_FALSE, glm::value_ptr( value ) );
        }
    }

    void OpenglState::GLShaderProgramSetMat4( const char * name, glm::mat4 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( value ) );
        }
    }

    void OpenglState::GLEnableAlphaBlending() {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    void OpenglState::GLEnablePreMultipliedAlphaBlending() {
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    }

    bool OpenglState::GLCheckShaderCompilationErrors( u32 shader ) {
        i32 success;
        char infoLog[ 1024 ];
        glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
        if( !success ) {
            glGetShaderInfoLog( shader, 1024, NULL, infoLog );
            core->LogOutput( LogLevel::ERR, "ERROR::SHADER_COMPILATION_ERROR of type: " );
            core->LogOutput( LogLevel::ERR, infoLog );
            core->LogOutput( LogLevel::ERR, "-- --------------------------------------------------- -- " );
        }

        return success;
    }

    bool OpenglState::GLCheckShaderLinkErrors( u32 program ) {
        i32 success;
        char infoLog[ 1024 ];
        glGetProgramiv( program, GL_LINK_STATUS, &success );
        if( !success ) {
            glGetProgramInfoLog( program, 1024, NULL, infoLog );
            core->LogOutput( LogLevel::ERR, "ERROR::SHADER_LINKER_ERROR of type: " );
            core->LogOutput( LogLevel::ERR, infoLog );
            core->LogOutput( LogLevel::ERR, "-- --------------------------------------------------- -- " );
        }

        return success;
    }

    GLShaderProgram OpenglState::GLCreateShaderProgram( const char * vertexSource, const char * fragmentSource ) {
        GLShaderProgram program = {};

        u32 vertexShader;
        vertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( vertexShader, 1, &vertexSource, NULL );
        glCompileShader( vertexShader );
        if( !GLCheckShaderCompilationErrors( vertexShader ) ) {
            return {};
        }

        u32 fragmentShader;
        fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( fragmentShader, 1, &fragmentSource, NULL );
        glCompileShader( fragmentShader );
        if( !GLCheckShaderCompilationErrors( fragmentShader ) ) {
            return {};
        }

        program.programHandle = glCreateProgram();
        glAttachShader( program.programHandle, vertexShader );
        glAttachShader( program.programHandle, fragmentShader );
        glLinkProgram( program.programHandle );
        if( !GLCheckShaderLinkErrors( program.programHandle ) ) {
            return {};
        }

        glDeleteShader( vertexShader );
        glDeleteShader( fragmentShader );

        return program;
    }

    GLVertexBuffer OpenglState::GLCreateVertexBuffer( VertexLayout * layout, i32 vertexCount, const void * srcData, bool dyanmic ) {
        GLVertexBuffer buffer = {};
        buffer.size = layout->SizeBytes() * vertexCount;
        buffer.stride = layout->StrideBytes();

        glGenVertexArrays( 1, &buffer.vao );
        glGenBuffers( 1, &buffer.vbo );

        glBindVertexArray( buffer.vao );

        glBindBuffer( GL_ARRAY_BUFFER, buffer.vbo );
        glBufferData( GL_ARRAY_BUFFER, buffer.size, srcData, dyanmic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW );

        layout->Layout();

        glBindVertexArray( 0 );

        return buffer;
    }

    void OpenglState::GLVertexBufferUpdate( GLVertexBuffer vertexBuffer, i32 offset, i32 size, const void * data ) {
        AssertMsg( vertexBuffer.size >= offset + size, "Vertex buffer update out of bounds" );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer.vbo );
        glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void OpenglState::GLVertexBufferUpdate( u32 vbo, i32 offset, i32 size, const void * data ) {
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void OpenglState::GLResetSurface( f32 w, f32 h, f32 cameraWidth, f32 cameraHeight ) {
        f32 ratioX = (f32)w / (f32)cameraWidth;
        f32 ratioY = (f32)h / (f32)cameraHeight;
        f32 ratio = ratioX < ratioY ? ratioX : ratioY;

        i32 viewWidth = (i32)( cameraWidth * ratio );
        i32 viewHeight = (i32)( cameraHeight * ratio );

        i32 viewX = (i32)( ( w - cameraWidth * ratio ) / 2 );
        i32 viewY = (i32)( ( h - cameraHeight * ratio ) / 2 );

        glViewport( viewX, viewY, viewWidth, viewHeight );

        core->mainSurfaceWidth = w;
        core->mainSurfaceHeight = h;
        core->viewport = glm::vec4( viewX, viewY, viewWidth, viewHeight );
        core->screenProjection = glm::ortho( 0.0f, (f32)w, 0.0f, h, -1.0f, 1.0f );
    }

    void OpenglState::GLInitializeShapeRendering() {
        const char * vertexShaderSource = R"(
            #version 330 core

            layout (location = 0) in vec2 position;

            uniform mat4 p;

            void main() {
                gl_Position = p * vec4(position.x, position.y, 0.0, 1.0);
            }
        )";

        const char * fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            uniform int  mode;
            uniform vec4 color;
            uniform vec4 shapePosAndSize;
            uniform vec4 shapeRadius;

            // from http://www.iquilezles.org/www/articles/distfunctions/distfunctions

            float CircleSDF(vec2 r, vec2 p, float rad) {
                return 1 - max(length(p - r) - rad, 0);
            }

            float BoxSDF(vec2 r, vec2 p, vec2 s) {
                return 1 - length(max(abs(p - r) - s, 0));
            }
            
            float RoundedBoxSDF(vec2 r, vec2 p, vec2 s, float rad) {
                return 1 - (length(max(abs(p - r) - s + rad, 0)) - rad);
            }

            void main() {
                vec2 s = shapePosAndSize.zw;
                vec2 r = shapePosAndSize.xy;
                vec2 p = gl_FragCoord.xy;

                if (mode == 0) {
                    FragColor = color;
                } else if (mode == 1) {
                    FragColor = color;
                } else if (mode == 2) {
                    float d = RoundedBoxSDF(r, p, s / 2, shapeRadius.x);
                    d = clamp(d, 0.0, 1.0);
                    FragColor = vec4(color.xyz, color.w * d);
                } else {
                    FragColor = vec4(1, 0, 1, 1);
                }
            }
        )";

        GLVertexLayoutShape shape = {};
        shapeProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        shapeVertexBuffer = GLCreateVertexBuffer( &shape, 96, nullptr, true );

        const i32 circleEdges = 32;
        float angleIncrement = 2.0f * PI / circleEdges;
        for( int i = 0; i < circleEdges; i++ ) {
            float angle1 = i * angleIncrement;
            float angle2 = ( i + 1 ) * angleIncrement;
            float x1 = cos( angle1 );
            float y1 = sin( angle1 );
            float x2 = cos( angle2 );
            float y2 = sin( angle2 );

            shapeCirleBase.Add( glm::vec2( 0.0f, 0.0f ) );
            shapeCirleBase.Add( glm::vec2( x1, y1 ) );
            shapeCirleBase.Add( glm::vec2( x2, y2 ) );
        }
    }

    void OpenglState::GLInitializeSpriteRendering() {
        const char * vertexShaderSource = R"(
            #version 330 core

            layout (location = 0) in vec2 position;
            layout (location = 1) in vec2 texCoord;

            out vec2 vertexTexCoord;

            uniform mat4 p;

            void main() {
                vertexTexCoord = texCoord;
                gl_Position = p * vec4(position.x, position.y, 0.0, 1.0);
            }
        )";

    #if 1
        const char * fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 vertexTexCoord;
            uniform vec4 color;
            uniform sampler2D texture0;

            vec2 uv_cstantos( vec2 uv, vec2 res ) {
                vec2 pixels = uv * res;

                // Updated to the final article
                vec2 alpha = 0.7 * fwidth(pixels);
                vec2 pixels_fract = fract(pixels);
                vec2 pixels_diff = clamp( .5 / alpha * pixels_fract, 0, .5 ) +
                                   clamp( .5 / alpha * (pixels_fract - 1) + .5, 0, .5 );
                pixels = floor(pixels) + pixels_diff;
                return pixels / res;
            }

            vec2 uv_klems( vec2 uv, ivec2 texture_size ) {
            
                vec2 pixels = uv * texture_size + 0.5;
    
                // tweak fractional value of the texture coordinate
                vec2 fl = floor(pixels);
                vec2 fr = fract(pixels);
                vec2 aa = fwidth(pixels) * 0.75;

                fr = smoothstep( vec2(0.5) - aa, vec2(0.5) + aa, fr);
    
                return (fl + fr - 0.5) / texture_size;
            }

            vec2 uv_iq( vec2 uv, ivec2 texture_size ) {
                vec2 pixel = uv * texture_size;

                vec2 seam = floor(pixel + 0.5);
                vec2 dudv = fwidth(pixel);
                pixel = seam + clamp( (pixel - seam) / dudv, -0.5, 0.5);
    
                return pixel / texture_size;
            }

            vec2 uv_fat_pixel( vec2 uv, ivec2 texture_size ) {
                vec2 pixel = uv * texture_size;

                vec2 fat_pixel = floor(pixel) + 0.5;
                // subpixel aa algorithm (COMMENT OUT TO COMPARE WITH POINT SAMPLING)
                fat_pixel += 1 - clamp((1.0 - fract(pixel)) * 3, 0, 1);
        
                return fat_pixel / texture_size;
            }

            vec2 uv_aa_smoothstep( vec2 uv, vec2 res, float width ) {
                uv = uv * res;
                vec2 uv_floor = floor(uv + 0.5);
                vec2 uv_fract = fract(uv + 0.5);
                vec2 uv_aa = fwidth(uv) * width * 0.5;
                uv_fract = smoothstep(
                    vec2(0.5) - uv_aa,
                    vec2(0.5) + uv_aa,
                    uv_fract
                    );
    
                return (uv_floor + uv_fract - 0.5) / res;
            }


            void main() {
                ivec2 tSize = textureSize(texture0, 0);
                //vec2 uv = uv_cstantos(vertexTexCoord, vec2(tSize.x, tSize.y));
                //vec2 uv = uv_iq(vertexTexCoord, tSize);
                //vec2 uv = uv_fat_pixel(vertexTexCoord, tSize);
                vec2 uv = uv_aa_smoothstep(vertexTexCoord, vec2(tSize.x, tSize.y), 1);
                vec4 sampled = texture(texture0, uv);
                //vec4 sampled = texture(texture0, vertexTexCoord);
                //sampled.rgb *= sampled.a;
                //if (sampled.a < 1) discard;
                FragColor = sampled * color;
            }
        )";
    #else 
        const char * fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 vertexTexCoord;
            uniform sampler2D texture0;
            uniform vec4 color;

            void main() {
                vec4 sampled = texture(texture0, vertexTexCoord) * color;
                FragColor = sampled;
            }
        )";
    #endif

        spriteProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );

        GLVertexLayoutSprite sprite = {};
        spriteVertexBuffer = GLCreateVertexBuffer( &sprite, 6, nullptr, true );
    }

    void OpenglState::GLInitializeUnlitModelRendering() {
        const char * vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;

            out vec2 TexCoords;

            uniform mat4 pvm;

            void main()
            {
                TexCoords = aTexCoords;
                gl_Position = pvm * vec4(aPos, 1.0);
            }
        )";


        const char * fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoords;

            uniform sampler2D texture0;
            uniform vec4 color;
            uniform int hasTexture;

            void main()
            {
                if ( hasTexture == 1 ) {
                    FragColor = texture(texture0, TexCoords);
                } else {
                    FragColor = color;
                    //FragColor = vec4(TexCoords.x, TexCoords.y, 0, 1) ;
                }
                //FragColor = color * vec4(TexCoords.x, TexCoords.y, 1, 1) ;
            }
        )";

        //staticMeshUnlitProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        //staticMeshTriangle = (Win32StaticMeshResource *)ResourceMeshCreate( "Triangle", 3 );
        //staticMeshPlane = (Win32StaticMeshResource *)ResourceMeshCreate( "Plane", StaticMeshGeneration::CreateQuad( -1, 1, 2.0f, 2.0f, 0.0f ) );
        //staticMeshBox = (Win32StaticMeshResource *)ResourceMeshCreate( "Box", StaticMeshGeneration::CreateBox( 1, 1, 1, 0 ) );
        //staticMeshSphere = (Win32StaticMeshResource *)ResourceMeshCreate( "Sphere", StaticMeshGeneration::CreateSphere( 1, 8, 8 ) );
    }

    TextureResource * OpenglState::ResourceGetAndCreateTexture( const char * name, bool genMips, bool genAnti ) {
        const i32 textureResourceCount = textures.GetCount();
        for( i32 i = 0; i < textureResourceCount; i++ ) {
            TextureResource & textureResource = textures[ i ];
            if( textureResource.name == name ) {
                return &textureResource;
            }
        }

        ContentTextureProcessor textureProcessor = {};
        bool loaded = textureProcessor.LoadFromFile( name );
        if( loaded == false ) {
            core->LogOutput( LogLevel::ERR, "Failed to load texture asset \t %s", name );
            return nullptr;
        }

        //textureProcessor.MakeAlphaEdge();
        textureProcessor.FixAplhaEdges();

        GLTextureResource textureResource = {};
        textureResource.id = StringHash::Hash( name );
        textureResource.name = name;
        textureResource.channels = textureProcessor.channels;
        textureResource.width = textureProcessor.width;
        textureResource.height = textureProcessor.height;
        textureResource.hasMips = genMips;
        textureResource.hasAnti = genAnti;

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // @TODO: Remove this pack the textures

        glGenTextures( 1, &textureResource.handle );
        glBindTexture( GL_TEXTURE_2D, textureResource.handle );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureResource.width, textureResource.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureProcessor.pixelData );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureResource.hasMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        if( textureResource.hasMips ) {
            glGenerateMipmap( GL_TEXTURE_2D );
        }

        if( textureResource.hasAnti ) {
            f32 maxAnti = 0.0f;
            glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnti );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAnti );
        }

        glBindTexture( GL_TEXTURE_2D, 0 );

        return textures.Add( textureResource );
    }


    void OpenglState::GLSetCamera( f32 width, f32 height ) {
        i32 w = 0;
        i32 h = 0;
        core->window.GetFramebufferSize( w, h );
        GLResetSurface( (f32)w, (f32)h, width, height );
    }

    void OpenglState::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
        if( clearBackBuffers ) {
            glClearColor( 0.5f, 0.2f, 0.2f, 1.0f );
            //glClearColor( 0.2f, 0.5f, 0.2f, 1.0f );
            //glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
            //glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            // Magenta
            //glClearColor( 1.0f, 0.0f, 1.0f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }

        f32 cameraWidth = dcxt->GetCameraWidth();
        f32 cameraHeight = dcxt->GetCameraHeight();
        GLSetCamera( cameraWidth, cameraHeight );

        const i32 drawCount = dcxt->drawList.GetCount();
        for( i32 i = 0; i < drawCount; i++ ) {
            DrawCommand & cmd = dcxt->drawList[ i ];
            switch( cmd.type ) {
                case DrawCommandType::CIRCLE:
                {
                    cmd.circle.c -= dcxt->cameraPos;

                    shapeCirle.Clear();
                    const i32 vertexCount = shapeCirleBase.GetCount();
                    for( i32 i = 0; i < vertexCount; i++ ) {
                        glm::vec2 pos = shapeCirleBase[ i ] * cmd.circle.r + cmd.circle.c;
                        shapeCirle.Add( pos );
                    }

                    GLEnableAlphaBlending();
                    GLShaderProgramBind( shapeProgram );
                    GLShaderProgramSetMat4( "p", cmd.proj );
                    GLShaderProgramSetInt( "mode", 1 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, shapeCirle.GetCount() * sizeof( glm::vec2 ), shapeCirle.GetData() );
                    glDrawArrays( GL_TRIANGLES, 0, shapeCirle.GetCount() );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::RECT:
                {
                    cmd.rect.bl -= dcxt->cameraPos;
                    cmd.rect.br -= dcxt->cameraPos;
                    cmd.rect.tr -= dcxt->cameraPos;
                    cmd.rect.tl -= dcxt->cameraPos;

                    f32 vertices[ 6 ][ 2 ] = {
                      { cmd.rect.tl.x, cmd.rect.tl.y, },
                      { cmd.rect.bl.x, cmd.rect.bl.y, },
                      { cmd.rect.br.x, cmd.rect.br.y, },

                      { cmd.rect.tl.x, cmd.rect.tl.y, },
                      { cmd.rect.br.x, cmd.rect.br.y, },
                      { cmd.rect.tr.x, cmd.rect.tr.y, }
                    };

                    GLEnableAlphaBlending();
                    GLShaderProgramBind( shapeProgram );
                    GLShaderProgramSetMat4( "p", cmd.proj );
                    GLShaderProgramSetInt( "mode", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::LINE2D:
                {
                    cmd.line2D.p1 -= dcxt->cameraPos;
                    cmd.line2D.p2 -= dcxt->cameraPos;
                    cmd.line2D.p3 -= dcxt->cameraPos;
                    cmd.line2D.p4 -= dcxt->cameraPos;

                    f32 vertices[ 6 ][ 2 ] = {
                         { cmd.line2D.p1.x, cmd.line2D.p1.y, },
                         { cmd.line2D.p2.x, cmd.line2D.p2.y, },
                         { cmd.line2D.p3.x, cmd.line2D.p3.y, },

                         { cmd.line2D.p3.x, cmd.line2D.p3.y, },
                         { cmd.line2D.p4.x, cmd.line2D.p4.y, },
                         { cmd.line2D.p1.x, cmd.line2D.p1.y, }
                    };

                    GLEnableAlphaBlending();
                    GLShaderProgramBind( shapeProgram );
                    GLShaderProgramSetMat4( "p", cmd.proj );
                    GLShaderProgramSetInt( "mode", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::TRIANGLE:
                {
                    cmd.triangle.p1 -= dcxt->cameraPos;
                    cmd.triangle.p2 -= dcxt->cameraPos;
                    cmd.triangle.p3 -= dcxt->cameraPos;

                    glm::vec2 vertices[ 3 ] = {
                        { cmd.triangle.p1 },
                        { cmd.triangle.p2 },
                        { cmd.triangle.p3 }
                    };

                    GLEnableAlphaBlending();
                    GLShaderProgramBind( shapeProgram );
                    GLShaderProgramSetMat4( "p", cmd.proj );
                    GLShaderProgramSetInt( "mode", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 3 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::TEXTURE:
                {
                    cmd.rect.bl -= dcxt->cameraPos;
                    cmd.rect.br -= dcxt->cameraPos;
                    cmd.rect.tr -= dcxt->cameraPos;
                    cmd.rect.tl -= dcxt->cameraPos;

                    GLTextureResource * texture = (GLTextureResource *)cmd.texture.textureRes;
                    AssertMsg( texture != nullptr, "Texture resource is null" );
                    /*
                        tl(0,1)  tr(1, 1)
                        bl(0,0)  br(1, 0)
                    */

                    f32 vertices[ 6 ][ 4 ] = {
                        { cmd.texture.tl.x, cmd.texture.tl.y, 0.0f ,0.0f },
                        { cmd.texture.bl.x, cmd.texture.bl.y, 0.0f, 1.0f },
                        { cmd.texture.br.x, cmd.texture.br.y, 1.0f, 1.0f },
                        { cmd.texture.tl.x, cmd.texture.tl.y, 0.0f, 0.0f },
                        { cmd.texture.br.x, cmd.texture.br.y, 1.0f, 1.0f },
                        { cmd.texture.tr.x, cmd.texture.tr.y, 1.0f, 0.0f }
                    };

                    //GLEnablePreMultipliedAlphaBlending();
                    GLEnableAlphaBlending();
                    GLShaderProgramBind( spriteProgram );
                    GLShaderProgramSetSampler( "texture0", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetTexture( 0, texture->handle );
                    GLShaderProgramSetMat4( "p", cmd.proj );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( spriteVertexBuffer.vao );
                    GLVertexBufferUpdate( spriteVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );

                } break;
                case DrawCommandType::SPRITE:
                {
                    cmd.rect.bl -= dcxt->cameraPos;
                    cmd.rect.br -= dcxt->cameraPos;
                    cmd.rect.tr -= dcxt->cameraPos;
                    cmd.rect.tl -= dcxt->cameraPos;

                    SpriteResource * spriteRes = cmd.sprite.spriteRes;
                    AssertMsg( spriteRes != nullptr, "Texture resource is null" );
                    GLTextureResource * texture = (GLTextureResource *)cmd.sprite.spriteRes->textureResource;
                    AssertMsg( texture != nullptr, "Texture resource is null" );

                    f32 vertices[ 6 ][ 4 ] = {
                       { cmd.sprite.tl.x, cmd.sprite.tl.y, cmd.sprite.tlUV.x ,cmd.sprite.tlUV.y },
                       { cmd.sprite.bl.x, cmd.sprite.bl.y, cmd.sprite.blUV.x, cmd.sprite.blUV.y },
                       { cmd.sprite.br.x, cmd.sprite.br.y, cmd.sprite.brUV.x, cmd.sprite.brUV.y },
                       { cmd.sprite.tl.x, cmd.sprite.tl.y, cmd.sprite.tlUV.x, cmd.sprite.tlUV.y },
                       { cmd.sprite.br.x, cmd.sprite.br.y, cmd.sprite.brUV.x, cmd.sprite.brUV.y },
                       { cmd.sprite.tr.x, cmd.sprite.tr.y, cmd.sprite.trUV.x, cmd.sprite.trUV.y }
                    };

                    //GLEnablePreMultipliedAlphaBlending();
                    GLEnableAlphaBlending();
                    GLShaderProgramBind( spriteProgram );
                    GLShaderProgramSetSampler( "texture0", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetTexture( 0, texture->handle );
                    GLShaderProgramSetMat4( "p", cmd.proj );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( spriteVertexBuffer.vao );
                    GLVertexBufferUpdate( spriteVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::TEXT:
                {
                    RenderDrawCommandText( cmd );
                } break;
                case DrawCommandType::PLANE:
                {
                    glm::vec3 up = glm::vec3( 0, 1, 0 );
                    f32 d = glm::dot( up, cmd.plane.normal );
                    if( d > 0.999f || d < -0.999f ) {
                        up = glm::vec3( 0, 0, 1 );
                    }

                    glm::mat4 m;
                    m[ 0 ] = glm::vec4( glm::cross( up, cmd.plane.normal ), 0 );
                    m[ 1 ] = glm::vec4( up, 0 );
                    m[ 2 ] = glm::vec4( cmd.plane.normal, 0 );
                    m[ 3 ] = glm::vec4( cmd.plane.center, 1 );

                    m = glm::scale( m, glm::vec3( cmd.plane.dim, 1.0f ) );

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v * m;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetInt( "hasTexture", 0 );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshPlane->vao );
                    glDrawElements( GL_TRIANGLES, staticMeshPlane->indexCount, GL_UNSIGNED_SHORT, 0 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::SPHERE:
                {
                    glm::mat4 m = glm::translate( glm::mat4( 1.0f ), cmd.sphere.center );
                    m = glm::scale( m, glm::vec3( cmd.sphere.r ) );

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v * m;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetInt( "hasTexture", 0 );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshSphere->vao );
                    glDrawElements( GL_TRIANGLES, staticMeshSphere->indexCount, GL_UNSIGNED_SHORT, 0 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::BOX:
                {
                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v * cmd.box.m;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetInt( "hasTexture", 0 );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    glDisable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshBox->vao );
                    glDrawElements( GL_TRIANGLES, staticMeshBox->indexCount, GL_UNSIGNED_SHORT, 0 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::LINE:
                {
                    StaticMeshVertex vertices[ 3 ] = {
                        { cmd.line.p1,      glm::vec3( 0 ), glm::vec2( 0 ) },
                        { cmd.line.p2,      glm::vec3( 0 ), glm::vec2( 0 ) },
                        { glm::vec3( 0 ),   glm::vec3( 0 ), glm::vec2( 0 ) },
                    };

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetMat4( "pvm", pvm );
                    GLShaderProgramSetInt( "hasTexture", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshTriangle->vao );
                    GLVertexBufferUpdate( staticMeshTriangle->vbo, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_LINES, 0, 2 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::TRIANGLE3D:
                {
                    StaticMeshVertex vertices[ 3 ] = {
                        { cmd.triangle3D.p1, glm::vec3( 0 ), cmd.triangle3D.uv1 },
                        { cmd.triangle3D.p2, glm::vec3( 0 ), cmd.triangle3D.uv2 },
                        { cmd.triangle3D.p3, glm::vec3( 0 ), cmd.triangle3D.uv3 }
                    };

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    if( cmd.triangle3D.texture != nullptr ) {
                        GLShaderProgramSetInt( "hasTexture", 1 );
                        GLTextureResource * texture = (GLTextureResource *)cmd.triangle3D.texture;
                        GLShaderProgramSetSampler( "texture0", 0 );
                        GLShaderProgramSetTexture( 0, texture->handle );
                    }
                    else {
                        GLShaderProgramSetInt( "hasTexture", 0 );
                        GLShaderProgramSetVec4( "color", cmd.color );
                    }

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshTriangle->vao );
                    GLVertexBufferUpdate( staticMeshTriangle->vbo, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 3 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::MESH:
                {
                    GLStaticMeshResource * mesh = (GLStaticMeshResource *)cmd.mesh.mesh;
                    Assert( mesh != nullptr );
                    if( mesh == nullptr ) {
                        break;
                    }

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v * cmd.mesh.m;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    if( cmd.mesh.albedo != nullptr ) {
                        GLShaderProgramSetInt( "hasTexture", 1 );
                        GLTextureResource * texture = (GLTextureResource *)cmd.mesh.albedo;
                        GLShaderProgramSetSampler( "texture0", 0 );
                        GLShaderProgramSetTexture( 0, texture->handle );
                    }
                    else {
                        GLShaderProgramSetInt( "hasTexture", 0 );
                        GLShaderProgramSetVec4( "color", cmd.color );
                    }

                    glDisable( GL_CULL_FACE ); // HACK:
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( mesh->vao );
                    glDrawElements( GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_SHORT, 0 );
                    glBindVertexArray( 0 );

                } break;
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }
        }
    }

    void OpenglState::Initialize( WindowsCore * core ) {
        this->core = core;

        gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress );
        core->LogOutput( LogLevel::INFO, "OpenGL %s, GLSL %s", glGetString( GL_VERSION ), glGetString( GL_SHADING_LANGUAGE_VERSION ) );

        GLSetCamera( 640, 360 );
        GLCheckCapablities();
        GLInitializeShapeRendering();
        GLInitializeSpriteRendering();
        GLInitializeTextRendering();
        GLInitializeUnlitModelRendering();
    }

    void WindowsCore::GLStart() {
        glState = new OpenglState();
        glState->Initialize( this );
    }
}

#endif