#include "../shared/atto_client.h"
#include "../game/atto_game.h"
#include "../shared/atto_colors.h"
#include "../shared/atto_mesh_generation.h"
#include "../content/atto_content.h"

#include "atto_core_windows.h"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <fstream>
#include "../editor/atto_editor.h"

namespace atto {

    static GLFWmonitor *               monitor = nullptr;
    static LargeString                 monitorName = LargeString::FromLiteral( "" );
    static f64                         monitorRefreshRate = 0;
    static GLFWwindow *                window = nullptr;
    static i32                         windowWidth = 0;
    static i32                         windowHeight = 0;
    static f32                         windowAspect = 0;
    static SmallString                 windowTitle = SmallString::FromLiteral( "Game" );
    static bool                        windowFullscreen = false;
    static bool                        shouldClose = false;
    static bool                        firstMouse = true;

    static void KeyCallback( GLFWwindow * window, int key, int scancode, int action, int mods ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        fi.keys[ key ] = action != GLFW_RELEASE;
    }

    static void MousePositionCallback( GLFWwindow * window, double xpos, double ypos ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        if( firstMouse ) {
            fi.lastMousePosPixels.x = (f32)xpos;
            fi.lastMousePosPixels.y = (f32)ypos;
            firstMouse = false;
        }
        else {
            fi.lastMousePosPixels = fi.mousePosPixels;
        }

        fi.mousePosPixels = glm::vec2( (f32)xpos, (f32)ypos );
        fi.mouseDeltaPixels.x = fi.mousePosPixels.x - fi.lastMousePosPixels.x;
        fi.mouseDeltaPixels.y = fi.lastMousePosPixels.y - fi.mousePosPixels.y;
    }

    static void MouseButtonCallback( GLFWwindow * window, int button, int action, int mods ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        fi.mouseButtons[ button ] = action != GLFW_RELEASE;
    }

    static void ScrollCallback( GLFWwindow * window, double xoffset, double yoffset ) {
        Core * core = (Core *)glfwGetWindowUserPointer( window );
        FrameInput & fi = core->InputGetFrameInput();
        fi.mouseWheelDelta = glm::vec2( (f32)xoffset, (f32)yoffset );
    }

    static void FramebufferCallback( GLFWwindow * window, i32 w, i32 h ) {
        WindowsCore * core = (WindowsCore *)glfwGetWindowUserPointer( window );
        windowWidth = w;
        windowHeight = h;
        windowAspect = (f32)w / (f32)h;

        core->GLResetSurface( (f32)w, (f32)h );
#if 0
        // Maintain aspect ratio with black bars
        mainSurfaceWidth = (i32)( 1280.0 * 1.6f );
        mainSurfaceHeight = (i32)( 720.0 * 1.6f );
        //mainSurfaceWidth = 480;
        //mainSurfaceHeight = 360;

        f32 ratioX = (f32)w / (f32)mainSurfaceWidth;
        f32 ratioY = (f32)h / (f32)mainSurfaceHeight;
        f32 ratio = ratioX < ratioY ? ratioX : ratioY;

        i32 viewWidth = (i32)( mainSurfaceWidth * ratio );
        i32 viewHeight = (i32)( mainSurfaceHeight * ratio );

        i32 viewX = (i32)( ( w - mainSurfaceWidth * ratio ) / 2 );
        i32 viewY = (i32)( ( h - mainSurfaceHeight * ratio ) / 2 );

        glViewport( viewX, viewY, viewWidth, viewHeight );
#endif
    }

    void WindowsCore::Run( int argc, char ** argv ) {
        OsParseStartArgs( argc, argv );

        MemoryMakePermanent( Megabytes( 128 ) );
        MemoryMakeTransient( Megabytes( 128 ) );

        if( !glfwInit() ) {
            //ATTOFATAL("Could not init GLFW, your windows is f*cked");
            return;
        }

        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );

#if ATTO_DEBUG_RENDERING
        glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
#endif

        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
        glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );
        //glfwWindowHint(GLFW_SAMPLES, 4);

        monitor = glfwGetPrimaryMonitor();
        if( monitor != nullptr ) {
            const GLFWvidmode * videoMode = glfwGetVideoMode( monitor );
            monitorRefreshRate = videoMode->refreshRate;
            monitorName = glfwGetMonitorName( monitor );
            //ATTOINFO("Using monitor name %s", os.monitorName.GetCStr());
        }

        windowWidth = theGameSettings.windowWidth;
        windowHeight = theGameSettings.windowHeight;

        window = glfwCreateWindow( windowWidth, windowHeight, windowTitle.GetCStr(), windowFullscreen ? monitor : nullptr, 0 );

        if( window == nullptr ) {
            LogOutput( LogLevel::FATAL, "Could not create window, your windows is f*cked" );
            return;
        }

        glfwMakeContextCurrent( window );
        glfwSwapInterval( 1 );

        glfwSetWindowUserPointer( window, this );

        glfwSetCursorPosCallback( window, MousePositionCallback );
        glfwSetKeyCallback( window, KeyCallback );
        glfwSetMouseButtonCallback( window, MouseButtonCallback );
        glfwSetScrollCallback( window, ScrollCallback );
        glfwSetFramebufferSizeCallback( window, FramebufferCallback );

        if( theGameSettings.windowStartPosX != -1 ) {
            glfwSetWindowPos( window, theGameSettings.windowStartPosX, theGameSettings.windowStartPosY );
        }

        gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress );
        LogOutput( LogLevel::INFO, "OpenGL %s, GLSL %s", glGetString( GL_VERSION ), glGetString( GL_SHADING_LANGUAGE_VERSION ) );

        i32 w = 0;
        i32 h = 0;
        glfwGetFramebufferSize( window, &w, &h );

        //RenderSetCamera( 320, 180 );
        //RenderSetCamera( 640, 360 );
        GLResetSurface( (f32)w, (f32)h );

        GLInitializeShapeRendering();
        GLInitializeSpriteRendering();
        GLInitializeTextRendering();
        GLInitializeUnlitModelRendering();

        ALInitialize();

        EngineImgui::Initialize( window );

        client = new NetClient( this );

        game = new Game();

    #if ATTO_EDITOR
        editor = new Editor();
    #endif
        
        bool showDemoWindow = true;

        f32 simTickRate = 1.0f / 30.0f;
        f32 simTickCurrent = 0.0f;

        this->deltaTime = 0;
        f64 startTime = glfwGetTime();
        while( !glfwWindowShouldClose( window ) ) {
            FrameInput & fi = InputGetFrameInput();
            fi.lastKeys = fi.keys;
            fi.lastMouseButtons = fi.mouseButtons;
            fi.mouseWheelDelta = glm::vec2( 0.0f, 0.0f );
            fi.mouseDeltaPixels = glm::vec2( 0.0f, 0.0f );

            glfwPollEvents();

            if( client->IsConnected() ) {
                NetworkMessage msg = {};
                while( client->Recieve( msg ) ) {
                    switch( msg.type ) {
                        case NetworkMessageType::NONE:
                            INVALID_CODE_PATH;
                            break;
                        case NetworkMessageType::GAME_START:
                        {

                        } break;
                        case NetworkMessageType::GGPO_MESSAGE:
                        {
                        } break;
                        default:
                            INVALID_CODE_PATH;
                            break;
                    }
                }
            }

        #if ATTO_EDITOR
            EngineImgui::NewFrame();
            editor->UpdateAndRender( this, game, deltaTime );
            EngineImgui::EndFrame();
        #else 
            game->UpdateAndRender( this, this->deltaTime );
        #endif

            glfwSwapBuffers( window );

            MemoryClearTransient();

            currentTime = glfwGetTime();
            f64 endTime = currentTime;
            this->deltaTime = (f32)( endTime - startTime );
            startTime = endTime;
        }

        EngineImgui::Shutdown();

        delete client;
    }

    StaticMeshResource * WindowsCore::ResourceMeshCreate( const char * name, StaticMeshData data ) {
        const i32 meshResourceCount = resources.meshes.GetCount();
        for( i32 i = 0; i < meshResourceCount; i++ ) {
            StaticMeshResource & meshResource = resources.meshes[ i ];
            if( meshResource.name == name ) {
                INVALID_CODE_PATH;
                return nullptr;
            }
        }

        Win32StaticMeshResource meshResource = {};
        meshResource.name = name;
        meshResource.vertexCount = data.vertexCount;
        meshResource.vertexStride = sizeof( StaticMeshVertex );
        meshResource.indexStride = sizeof( u16 );
        meshResource.indexCount = data.indexCount;
        
        glGenVertexArrays( 1, &meshResource.vao );
        glGenBuffers( 1, &meshResource.vbo );
        glGenBuffers( 1, &meshResource.ibo );

        glBindVertexArray( meshResource.vao );
        glBindBuffer( GL_ARRAY_BUFFER, meshResource.vbo );
        glBufferData( GL_ARRAY_BUFFER, data.vertexCount * sizeof( StaticMeshVertex ), data.vertices, GL_STATIC_DRAW );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, meshResource.ibo );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, data.indexCount * sizeof( u16 ), data.indices, GL_STATIC_DRAW );

        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)0 );

        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, normal ) );

        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, uv ) );

        glBindVertexArray( 0 );

        return resources.meshes.Add( meshResource );
    }

    void WindowsCore::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
        if( clearBackBuffers ) {
            //glClearColor(0.5f, 0.2f, 0.2f, 1.0f);
            glClearColor(0.2f, 0.5f, 0.2f, 1.0f);
            //glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
            //glClearColor( Colors::SILVER.x, Colors::SILVER.y, Colors::SILVER.z, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }

        const i32 drawCount = dcxt->drawList.GetCount();
        for( i32 i = 0; i < drawCount; i++ ) {
            DrawCommand & cmd = dcxt->drawList[ i ];
            switch( cmd.type ) {
                case DrawCommandType::CIRCLE:
                {
                    //cmd.circle.c = WorldPosToScreenPos( cmd.circle.c );
                    //cmd.circle.r = WorldLengthToScreenLength( cmd.circle.r );

                    f32 x1 = cmd.circle.c.x - cmd.circle.r;
                    f32 y1 = cmd.circle.c.y - cmd.circle.r;
                    f32 x2 = cmd.circle.c.x + cmd.circle.r;
                    f32 y2 = cmd.circle.c.y + cmd.circle.r;

                    f32 vertices[ 6 ][ 2 ] = {
                        { x1, y2 },
                        { x1, y1 },
                        { x2, y1 },
                        { x1, y2 },
                        { x2, y1 },
                        { x2, y2 }
                    };

                    GLEnableAlphaBlending();
                    GLShaderProgramBind( shapeProgram );
                    GLShaderProgramSetMat4( "p", screenProjection );
                    GLShaderProgramSetInt( "mode", 1 );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    //GLShaderProgramSetVec4( "shapePosAndSize", glm::vec4( cmd.circle.c.x, (f32)viewport.w - cmd.circle.c.y, cmd.circle.r, cmd.circle.r ) );
                    GLShaderProgramSetVec4( "shapeRadius", glm::vec4( cmd.circle.r - 2, 0, 0, 0 ) ); // The 4 here is to stop the circle from being cut of from the edges

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::RECT:
                {
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
                    GLShaderProgramSetMat4( "p", cmd.projection );
                    GLShaderProgramSetInt( "mode", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );

                    glDisable( GL_CULL_FACE );
                    glBindVertexArray( shapeVertexBuffer.vao );
                    GLVertexBufferUpdate( shapeVertexBuffer, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 6 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::SPRITE:
                {
                    Win32TextureResource * texture = (Win32TextureResource *)cmd.sprite.textureRes;
                    AssertMsg( texture != nullptr, "Texture resource is null" );
                    /*
                        tl(0,1)  tr(1, 1)
                        bl(0,0)  br(1, 0)
                    */

                    f32 vertices[ 6 ][ 4 ] = {
                        { cmd.sprite.tl.x, cmd.sprite.tl.y, 0.0f ,0.0f },
                        { cmd.sprite.bl.x, cmd.sprite.bl.y, 0.0f, 1.0f },
                        { cmd.sprite.br.x, cmd.sprite.br.y, 1.0f, 1.0f },
                        { cmd.sprite.tl.x, cmd.sprite.tl.y, 0.0f, 0.0f },
                        { cmd.sprite.br.x, cmd.sprite.br.y, 1.0f, 1.0f },
                        { cmd.sprite.tr.x, cmd.sprite.tr.y, 1.0f, 0.0f }
                    };

                    //GLEnablePreMultipliedAlphaBlending();
                    GLEnableAlphaBlending();
                    GLShaderProgramBind( spriteProgram );
                    GLShaderProgramSetSampler( "texture0", 0 );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetTexture( 0, texture->handle );
                    GLShaderProgramSetMat4( "p", cmd.projection );

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
                    glm::mat4 p = dcxt->cameraProj;
                    glm::mat4 pvm = p * v * m;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetVec4( "color", cmd.color );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );

                    glBindVertexArray( staticMeshPlane->vao );
                    glDrawElements( GL_TRIANGLES, staticMeshPlane->indexCount, GL_UNSIGNED_SHORT, 0 );
                    glBindVertexArray( 0 );

                } break;
                default:
                {
                    //ATTOASSERT(false, "Invalid draw command type");
                } break;
            }
        }
    }

    TextureResource * WindowsCore::ResourceGetAndLoadTexture( const char * name ) {
        const i32 textureResourceCount = resources.textures.GetCount();
        for( i32 i = 0; i < textureResourceCount; i++ ) {
            TextureResource & textureResource = resources.textures[ i ];
            if( textureResource.name == name ) {
                return &textureResource;
            }
        }

        ContentTextureProcesor textureProcessor = {};
        textureProcessor.LoadFromFile( name );
        textureProcessor.MakeAlphaEdge();
        textureProcessor.FixAplhaEdges();

        Win32TextureResource textureResource = {};
        textureResource.name = name;
        textureResource.channels = textureProcessor.channels;
        textureResource.width = textureProcessor.width;
        textureResource.height = textureProcessor.height;
        textureResource.generateMipMaps = false;

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // @TODO: Remove this pack the textures

        glGenTextures( 1, &textureResource.handle );
        glBindTexture( GL_TEXTURE_2D, textureResource.handle );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureResource.width, textureResource.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureProcessor.pixelData );
        if( textureResource.generateMipMaps ) {
            glGenerateMipmap( GL_TEXTURE_2D );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureResource.generateMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glBindTexture( GL_TEXTURE_2D, 0 );

        return resources.textures.Add( textureResource );
    }

    void WindowsCore::InputDisableMouse() {
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    }

    void WindowsCore::InputEnableMouse() {
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    }

    bool WindowsCore::InputIsMouseDisabled() {
        return glfwGetInputMode( window, GLFW_CURSOR ) == GLFW_CURSOR_DISABLED;
    }

    void WindowsCore::WindowClose() {
        glfwSetWindowShouldClose( window, true );
    }

    void WindowsCore::WindowSetTitle( const char * title ) {
        glfwSetWindowTitle( window, title );
    }

    void WindowsCore::GLInitializeShapeRendering() {
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
                    float d = CircleSDF(r, p, shapeRadius.x);
                    d = clamp(d, 0.0, 1.0);
                    FragColor = color * d; //vec4(color.xyz, color.w * d);
                } else if (mode == 2) {
                    float d = RoundedBoxSDF(r, p, s / 2, shapeRadius.x);
                    d = clamp(d, 0.0, 1.0);
                    FragColor = vec4(color.xyz, color.w * d);
                } else {
                    FragColor = vec4(1, 0, 1, 1);
                }
            }
        )";

        VertexLayoutShape shape = {};
        shapeProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        shapeVertexBuffer = GLCreateVertexBuffer( &shape, 6, nullptr, true );
    }

    void WindowsCore::GLInitializeSpriteRendering() {
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

#if 0
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
                vec2 uv = uv_fat_pixel(vertexTexCoord, tSize);
                //vec2 uv = uv_aa_smoothstep(vertexTexCoord, vec2(tSize.x, tSize.y), 1);
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
        
        VertexLayoutSprite sprite = {};
        spriteVertexBuffer = GLCreateVertexBuffer( &sprite, 6, nullptr, true );
    }

    void WindowsCore::GLInitializeUnlitModelRendering() {
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

            void main()
            {
                //FragColor = texture(texture0, TexCoords) * color;
                FragColor = color;
            }
        )";

        staticMeshUnlitProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        staticMeshPlane     = (Win32StaticMeshResource *)ResourceMeshCreate( "Plane",   StaticMeshGeneration::CreateQuad( -1, 1, 2.0f, 2.0f, 0.0f ) );
        staticMeshCube      = (Win32StaticMeshResource *)ResourceMeshCreate( "Cube",    StaticMeshGeneration::CreateBox( 1, 1, 1, 0 ) );
        staticMeshSphere    = (Win32StaticMeshResource *)ResourceMeshCreate( "Sphere",  StaticMeshGeneration::CreateSphere( 1, 8, 8 ) );
    }

    void WindowsCore::OsParseStartArgs( int argc, char ** argv ) {
        if( argc > 1 ) {
            GameSettings settings = {};
            //REFL_READ_JSON( argv[ 1 ], GameSettings, settings );
            theGameSettings = settings;
        }
        else {
            GameSettings settings = {};
            settings.basePath = "assets/";
            settings.windowStartPosX = -1;
            settings.windowStartPosY = -1;
            //settings.windowWidth = 16 * 115;
            //settings.windowHeight = 9 * 115;
            settings.windowWidth = 1280;
            settings.windowHeight = 720;
            //settings.windowWidth = 1920;
            //settings.windowHeight = 1050;
            settings.fullscreen = false;
            settings.vsync = true;
            settings.showDebug = true;
            settings.noAudio = false;

            theGameSettings = settings;
        }
    }

    void WindowsCore::GLShaderProgramBind( ShaderProgram & program ) {
        AssertMsg( program.programHandle != 0, "Shader program not created" );
        glUseProgram( program.programHandle );
        boundProgram = &program;
    }

    i32 WindowsCore::GLShaderProgramGetUniformLocation( ShaderProgram & program, const char * name ) {
        if( program.programHandle == 0 ) {
            LogOutput( LogLevel::ERR, "Shader program in not valid" );
            return -1;
        }

        const u32 uniformCount = program.uniforms.GetCount();
        for( u32 uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++ ) {
            ShaderUniform & uniform = program.uniforms[ uniformIndex ];
            if( uniform.name == name ) {
                return uniform.location;
            }
        }

        i32 location = glGetUniformLocation( program.programHandle, name );
        if( location >= 0 ) {
            ShaderUniform newUniform = {};
            newUniform.location = location;
            newUniform.name = name;

            program.uniforms.Add( newUniform );
        }
        else {
            LogOutput( LogLevel::ERR, "Could not find uniform value %s", name );
        }

        return location;
    }

    void WindowsCore::GLShaderProgramSetInt( const char * name, i32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1i( location, value );
        }
    }

    void WindowsCore::GLShaderProgramSetSampler( const char * name, i32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1i( location, value );
        }
    }

    void WindowsCore::GLShaderProgramSetTexture( i32 location, u32 textureHandle ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        glBindTextureUnit( 0, textureHandle );
    }

    void WindowsCore::GLShaderProgramSetTexture( const char * name, u32 textureHandle, i32 slot ) {
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        glActiveTexture( GL_TEXTURE0 + slot );
        glBindTexture( GL_TEXTURE_2D, textureHandle );
        glUniform1i( location, slot );
    }

    void WindowsCore::GLShaderProgramSetFloat( const char * name, f32 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform1f( location, value );
        }
    }

    void WindowsCore::GLShaderProgramSetVec2( const char * name, glm::vec2 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform2fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void WindowsCore::GLShaderProgramSetVec3( const char * name, glm::vec3 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform3fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void WindowsCore::GLShaderProgramSetVec4( const char * name, glm::vec4 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniform4fv( location, 1, glm::value_ptr( value ) );
        }
    }

    void WindowsCore::GLShaderProgramSetMat3( const char * name, glm::mat3 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniformMatrix3fv( location, 1, GL_FALSE, glm::value_ptr( value ) );
        }
    }

    void WindowsCore::GLShaderProgramSetMat4( const char * name, glm::mat4 value ) {
        AssertMsg( boundProgram != nullptr, "No shader program bound" );
        i32 location = GLShaderProgramGetUniformLocation( *boundProgram, name );
        if( location >= 0 ) {
            glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( value ) );
        }
    }

    void WindowsCore::GLEnableAlphaBlending() {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    void WindowsCore::GLEnablePreMultipliedAlphaBlending() {
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    }

    bool WindowsCore::GLCheckShaderCompilationErrors( u32 shader ) {
        i32 success;
        char infoLog[ 1024 ];
        glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
        if( !success ) {
            glGetShaderInfoLog( shader, 1024, NULL, infoLog );
            LogOutput( LogLevel::ERR, "ERROR::SHADER_COMPILATION_ERROR of type: " );
            LogOutput( LogLevel::ERR, infoLog );
            LogOutput( LogLevel::ERR, "-- --------------------------------------------------- -- " );
        }

        return success;
    }

    bool WindowsCore::GLCheckShaderLinkErrors( u32 program ) {
        i32 success;
        char infoLog[ 1024 ];
        glGetProgramiv( program, GL_LINK_STATUS, &success );
        if( !success ) {
            glGetProgramInfoLog( program, 1024, NULL, infoLog );
            LogOutput( LogLevel::ERR, "ERROR::SHADER_LINKER_ERROR of type: " );
            LogOutput( LogLevel::ERR, infoLog );
            LogOutput( LogLevel::ERR, "-- --------------------------------------------------- -- " );
        }

        return success;
    }

    ShaderProgram WindowsCore::GLCreateShaderProgram( const char * vertexSource, const char * fragmentSource ) {
        ShaderProgram program = {};

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

    VertexBuffer WindowsCore::GLCreateVertexBuffer( VertexLayout * layout, i32 vertexCount, const void * srcData, bool dyanmic ) {
        VertexBuffer buffer = {};
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

    void WindowsCore::GLVertexBufferUpdate( VertexBuffer vertexBuffer, i32 offset, i32 size, const void * data ) {
        AssertMsg( vertexBuffer.size >= offset + size, "Vertex buffer update out of bounds" );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer.vbo );
        glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void WindowsCore::GLResetSurface( f32 w, f32 h ) {
        // Maintain aspect ratio with black bars
        //f32 ratioX = (f32)w / (f32)cameraWidth;
        //f32 ratioY = (f32)h / (f32)cameraHeight;
        //f32 ratio = ratioX < ratioY ? ratioX : ratioY;
        //
        //i32 viewWidth = (i32)( cameraWidth * ratio );
        //i32 viewHeight = (i32)( cameraHeight * ratio );
        //
        //i32 viewX = (i32)( ( w - cameraWidth * ratio ) / 2 );
        //i32 viewY = (i32)( ( h - cameraHeight * ratio ) / 2 );
        //
        //glViewport( viewX, viewY, viewWidth, viewHeight );
        //
        mainSurfaceWidth = w;
        mainSurfaceHeight = h;
        //viewport = glm::vec4( viewX, viewY, viewWidth, viewHeight );
        //cameraProjection = glm::ortho( 0.0f, (f32)cameraWidth, 0.0f, (f32)cameraHeight, -1.0f, 1.0f );

        glViewport( 0, 0, (i32)w, (i32)h );
        screenProjection = glm::ortho( 0.0f, (f32)w, (f32)h, 0.0f, -1.0f, 1.0f );
    }

    void VertexLayoutShape::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, false, stride, 0 );
    }

    i32 VertexLayoutShape::SizeBytes() {
        return (i32)sizeof( ShapeVertex );
    }

    i32 VertexLayoutShape::StrideBytes() {
        return (i32)sizeof( ShapeVertex );
    }

    void VertexLayoutSprite::Layout() {
        i32 stride = StrideBytes();
        glEnableVertexAttribArray( 0 );
        glEnableVertexAttribArray( 1 );
        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, false, stride, 0 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, false, stride, (void *)( 2 * sizeof( f32 ) ) );
    }

    i32 VertexLayoutSprite::SizeBytes() {
        return (i32)sizeof( SpriteVertex );
    }

    i32 VertexLayoutSprite::StrideBytes() {
        return (i32)sizeof( SpriteVertex );
    }

    void VertexLayoutStaticModel::Layout() {
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)0 );
        
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, normal ) );
        
        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( StaticMeshVertex ), (void *)offsetof( StaticMeshVertex, uv ) );
    }

    i32 VertexLayoutStaticModel::SizeBytes() {
        return (i32)sizeof( StaticMeshVertex );
    }

    i32 VertexLayoutStaticModel::StrideBytes() {
        return (i32)sizeof( StaticMeshVertex );
    }

}

