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
#include "../game/modes/atto_game_mode_game.h"
#include "../game/modes/atto_game_mode_main_menu.h"

namespace atto {

    static GLFWmonitor * monitor = nullptr;
    static LargeString                 monitorName = LargeString::FromLiteral( "" );
    static f64                         monitorRefreshRate = 0;
    static GLFWwindow *                window = nullptr;
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
        // We need to += here because this callback can be called multiple times when calling glfwPollEvents. So we need to collect all the inputs.
        fi.mouseDeltaPixels.x += fi.mousePosPixels.x - fi.lastMousePosPixels.x;
        fi.mouseDeltaPixels.y += fi.lastMousePosPixels.y - fi.mousePosPixels.y;
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
    }

    void WindowsCore::Run( int argc, char ** argv ) {
        theCore = this;

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
        glfwWindowHint(GLFW_SAMPLES, 4);

        monitor = glfwGetPrimaryMonitor();
        if( monitor != nullptr ) {
            const GLFWvidmode * videoMode = glfwGetVideoMode( monitor );
            monitorRefreshRate = videoMode->refreshRate;
            monitorName = glfwGetMonitorName( monitor );
            //ATTOINFO("Using monitor name %s", os.monitorName.GetCStr());
        }

        i32 windowWidth = theGameSettings.windowWidth;
        i32 windowHeight = theGameSettings.windowHeight;
        //windowFullscreen = true;
        window = glfwCreateWindow( windowWidth, windowHeight, windowTitle.GetCStr(), windowFullscreen ? monitor : nullptr, 0 );

        if( window == nullptr ) {
            LogOutput( LogLevel::FATAL, "Could not create window, your windows is f*cked" );
            return;
        }

        glfwMakeContextCurrent( window );
        glfwSwapInterval( theGameSettings.vsync );

        glfwSetWindowUserPointer( window, this );

        if( glfwRawMouseMotionSupported() ) {
            glfwSetInputMode( window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
        }
        else {
            LogOutput( LogLevel::WARN, "Could not enable raw input" );
        }

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

        //RenderSetCameraDims( 320, 180 );
        //RenderSetCamera( 320 * 1.5f, 180 * 1.5f );
        GLSetCamera( 640, 360 );
        //RenderSetCamera( 1280, 720 );
        GLCheckCapablities();
        GLInitializeShapeRendering();
        GLInitializeSpriteRendering();
        GLInitializeTextRendering();
        GLInitializeUnlitModelRendering();

        AudioInitialize();

        taskScheduler.Initialize( 4 );

    #if ATTO_EDITOR
        EngineImgui::Initialize( window );
        editor = new Editor();
        editor->Initialize( this );
    #endif
        
        client = new NetClient( this );

        //game = new GameModeGame();
        currentGameMode = new GameMode_MainMenu();
        currentGameMode->Initialize( this );
        
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

            //std::cout << "Poll start " << std::endl;
            glfwPollEvents();
            //std::cout << "Poll end " << std::endl;

            if( InputKeyJustPressed( KEY_CODE_ENTER ) && InputKeyDown( KEY_CODE_LEFT_ALT ) ) {
                if( windowFullscreen ) {
                    glfwSetWindowMonitor( window, nullptr, theGameSettings.windowStartPosX, theGameSettings.windowStartPosY, windowWidth, windowHeight, 0 );
                    windowFullscreen = false;
                }
                else {
                    const GLFWvidmode * mode = glfwGetVideoMode( monitor );
                    glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
                    windowFullscreen = true;
                }
            }


            bool skipFrame = false;
            if( nextGameMode != nullptr ) {
                currentGameMode->Shutdown( this );
                delete currentGameMode;
                currentGameMode = nextGameMode;
                nextGameMode = nullptr;
                currentGameMode->Initialize( this );
                skipFrame = true;
            }

        #if ATTO_EDITOR
            editor->UpdateAndRender( this, currentGameMode, deltaTime );
        #else 
            if( skipFrame == false ) {
                if( currentGameMode->IsInitialized() == false ) {
                    currentGameMode->Initialize( this );
                    return;
                }

                currentGameMode->UpdateAndRender( this, this->deltaTime );
            }
        #endif

            AudioUpdate();

            glfwSwapBuffers( window );

            MemoryClearTransient();

            f64 endTime = glfwGetTime();
            this->deltaTime = (f32)( endTime - startTime );
            startTime = endTime;
        }
        
    #if ATTO_EDITOR
        EngineImgui::Shutdown();
    #endif

        delete client;
    }

    f64 WindowsCore::GetTheCurrentTime() const {
        return glfwGetTime();
    }

    void WindowsCore::GLSetCamera( f32 width, f32 height ) {
        i32 w = 0;
        i32 h = 0;
        glfwGetFramebufferSize( window, &w, &h );
        GLResetSurface( (f32)w, (f32)h, width, height );
    }

    void WindowsCore::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
        if( clearBackBuffers ) {
            glClearColor(0.5f, 0.2f, 0.2f, 1.0f);
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
                    for ( i32 i = 0; i < vertexCount; i++ ) {
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
                case DrawCommandType::TEXTURE:
                {
                    cmd.rect.bl -= dcxt->cameraPos;
                    cmd.rect.br -= dcxt->cameraPos;
                    cmd.rect.tr -= dcxt->cameraPos;
                    cmd.rect.tl -= dcxt->cameraPos;

                    Win32TextureResource * texture = (Win32TextureResource *)cmd.texture.textureRes;
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
                    Win32TextureResource * texture = (Win32TextureResource *)cmd.sprite.spriteRes->textureResource;
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
                    GetVertexBufferUpdate( staticMeshTriangle->vbo, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_LINES, 0, 2 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::TRIANGLE:
                {
                    StaticMeshVertex vertices[ 3 ] = {
                        { cmd.triangle.p1, glm::vec3( 0 ), cmd.triangle.uv1 },
                        { cmd.triangle.p2, glm::vec3( 0 ), cmd.triangle.uv2 },
                        { cmd.triangle.p3, glm::vec3( 0 ), cmd.triangle.uv3 }
                    };

                    glm::mat4 v = dcxt->cameraView;
                    glm::mat4 p = dcxt->cameraProjection;
                    glm::mat4 pvm = p * v;

                    GLShaderProgramBind( staticMeshUnlitProgram );
                    GLShaderProgramSetMat4( "pvm", pvm );

                    if( cmd.triangle.texture != nullptr ) {
                        GLShaderProgramSetInt( "hasTexture", 1 );
                        Win32TextureResource * texture = (Win32TextureResource *)cmd.triangle.texture;
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
                    GetVertexBufferUpdate( staticMeshTriangle->vbo, 0, sizeof( vertices ), vertices );
                    glDrawArrays( GL_TRIANGLES, 0, 3 );
                    glBindVertexArray( 0 );
                } break;
                case DrawCommandType::MESH:
                {
                    Win32StaticMeshResource * mesh = (Win32StaticMeshResource *)cmd.mesh.mesh;
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
                        Win32TextureResource * texture = (Win32TextureResource *)cmd.mesh.albedo;
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
                    //ATTOASSERT(false, "Invalid draw command type");
                } break;
            }
        }
    }

    TextureResource * WindowsCore::ResourceGetAndCreateTexture( const char * name, bool genMips, bool genAnti ) {
        const i32 textureResourceCount = resources.textures.GetCount();
        for( i32 i = 0; i < textureResourceCount; i++ ) {
            TextureResource & textureResource = resources.textures[ i ];
            if( textureResource.name == name ) {
                return &textureResource;
            }
        }

        ContentTextureProcessor textureProcessor = {};
        bool loaded = textureProcessor.LoadFromFile( name );
        if( loaded == false ) {
            LogOutput( LogLevel::ERR, "Failed to load texture asset \t %s", name );
            return nullptr;
        }

        //textureProcessor.MakeAlphaEdge();
        textureProcessor.FixAplhaEdges();

        Win32TextureResource textureResource = {};
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

        // Generate mipsmaps, thanks opengl for making this easy.
        if( textureResource.hasMips ) {
            glGenerateMipmap( GL_TEXTURE_2D );
        }

        if( textureResource.hasAnti ) {
            f32 maxAnti = 0.0f;
            glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnti );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAnti );
        }

        glBindTexture( GL_TEXTURE_2D, 0 );

        return resources.textures.Add( textureResource );

    }

    TextureResource * WindowsCore::ResourceGetAndLoadTexture( const char * name ) {
        return ResourceGetAndCreateTexture( name, false , false );
        //const i32 textureResourceCount = resources.textures.GetCount();
        //for( i32 i = 0; i < textureResourceCount; i++ ) {
        //    TextureResource & textureResource = resources.textures[ i ];
        //    if( textureResource.name == name ) {
        //        return &textureResource;
        //    }
        //}
        //
        //Win32TextureResource * textureResource = MemoryAllocateTransient<Win32TextureResource>();
        //if( ResourceReadTextRefl<TextureResource>( (TextureResource *)textureResource, name ) == true ) {
        //    
        //}
        //return nullptr;
    }

    inline static SmallString GetEndingFolder( const char * path ) {
        SmallString folder = {};
        i32 lastSlash = -1;
        for( i32 i = 0; path[ i ] != '\0'; i++ ) {
            if( path[ i ] == '/' ) {
                lastSlash = i;
            }
        }

        if( lastSlash != -1 ) {
            folder.Add( path + lastSlash + 1 );
        }
        else {
            folder.Add( path );
        }

        return folder;
    }

    SpriteResource * WindowsCore::ResourceGetAndCreateSprite( const char * spriteName, i32 frameCount, i32 frameWidth, i32 frameHeight, i32 frameRate ) {
        const i32 spriteResourceCount = resources.sprites.GetCount();
        for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
            SpriteResource & sprite = resources.sprites[ spriteIndex ];
            if( sprite.spriteName == spriteName ) {
                return &sprite;
            }
        }

        LargeString textureName = LargeString::FromLiteral( spriteName );
        textureName.StripFileExtension();
        textureName.Add( ".png" );
        TextureResource * textureResource = ResourceGetAndCreateTexture( textureName.GetCStr(), false, false );
        if( textureResource == nullptr ) {
            LogOutput( LogLevel::ERR, "Could not find texture for sprite '%s'", spriteName );
        }

        SpriteResource * spriteResource = MemoryAllocateTransient<SpriteResource>();
        spriteResource->spriteId = (i64)StringHash::Hash( spriteName );
        spriteResource->spriteName = spriteName;
        spriteResource->textureResource = textureResource;
        spriteResource->frameCount = frameCount;
        spriteResource->frameWidth = frameWidth;
        spriteResource->frameHeight = frameHeight;
        spriteResource->frameRate = frameRate;

    #if 0
        LargeString tName = StringFormat::Large( "%res/sprites/%s/%s.json", spriteName, shortName.GetCStr() );
        ResourceSaveToFile( this, spriteResource, tName.GetCStr() );
    #endif

        return resources.sprites.Add_MemCpyPtr( spriteResource );
    }

    SpriteResource * WindowsCore::ResourceGetAndLoadSprite( const char * spriteName ) {
        const i32 spriteResourceCount = resources.sprites.GetCount();
        for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
            SpriteResource & sprite = resources.sprites[ spriteIndex ];
            if( sprite.spriteName == spriteName ) {
                return &sprite;
            }
        }

        SpriteResource * spriteResource = MemoryAllocateTransient<SpriteResource>();
        spriteResource->spriteName = spriteName;

        if( ResourceReadTextRefl( spriteResource, spriteName ) == true ) {
            return resources.sprites.Add_MemCpyPtr( spriteResource );
        }

        return nullptr;
    }

    SpriteResource * WindowsCore::ResourceGetLoadedSprite( i64 spriteId ) {
        const i32 spriteResourceCount = resources.sprites.GetCount();
        for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
            SpriteResource & sprite = resources.sprites[ spriteIndex ];
            if( sprite.spriteId == spriteId ) {
                return &sprite;
            }
        }

        INVALID_CODE_PATH;

        return nullptr;
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

    void WindowsCore::WindowSetVSync( bool value ) {
        glfwSwapInterval( (i32)value );
        theGameSettings.vsync = value;
    }

    bool WindowsCore::WindowGetVSync() {
        return theGameSettings.vsync;
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

        VertexLayoutShape shape = {};
        shapeProgram = GLCreateShaderProgram( vertexShaderSource, fragmentShaderSource );
        shapeVertexBuffer = GLCreateVertexBuffer( &shape, 96, nullptr, true );

        const i32 circleEdges = 32;
        float angleIncrement = 2.0f * PI / circleEdges;
        for (int i = 0; i < circleEdges; i++ ) {
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

    void WindowsCore::OsParseStartArgs( int argc, char ** argv ) {
       
        if( argc > 1 ) {
            LogOutput( LogLevel::TRACE, "Parsing start up settings" );

            // TODO(DECLAN): Don't do this...
            static char buffer[ 2048 ] = {};
            ResourceReadEntireTextFile( argv[ 1 ], buffer, sizeof( buffer ) );
            
            nlohmann::json j = nlohmann::json::parse( buffer );
            GameSettings settings = GameSettings::CreateSensibleDefaults();
            TypeDescriptor * settingsType = TypeResolver<GameSettings>::get();
            settingsType->JSON_Read( j, &settings );
            theGameSettings = settings;
        }
        else {
            theGameSettings = GameSettings::CreateSensibleDefaults();
        }
    }

    void WindowsCore::GLCheckCapablities() {
        // I was going to check for antisotrophic filtering here but apparently this is standard in opengl 4.6 which makes sense and come one, which GPU won't have this ?
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

    void WindowsCore::GetVertexBufferUpdate( u32 vbo, i32 offset, i32 size, const void * data ) {
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void WindowsCore::GLResetSurface( f32 w, f32 h, f32 cameraWidth, f32 cameraHeight ) {
        f32 ratioX = (f32)w / (f32)cameraWidth;
        f32 ratioY = (f32)h / (f32)cameraHeight;
        f32 ratio = ratioX < ratioY ? ratioX : ratioY;

        i32 viewWidth = (i32)( cameraWidth * ratio );
        i32 viewHeight = (i32)( cameraHeight * ratio );

        i32 viewX = (i32)( ( w - cameraWidth * ratio ) / 2 );
        i32 viewY = (i32)( ( h - cameraHeight * ratio ) / 2 );

        glViewport( viewX, viewY, viewWidth, viewHeight );

        mainSurfaceWidth = w;
        mainSurfaceHeight = h;
        viewport = glm::vec4( viewX, viewY, viewWidth, viewHeight );
        screenProjection = glm::ortho( 0.0f, (f32)w, 0.0f, h, -1.0f, 1.0f );
    }

#if ATTO_EDITOR

    template<typename _type_, typename _base_ >
    void WriteResourceArray( BinaryBlob & blob, FixedList<_type_, 1024> & resources ) {
        const i32 count = resources.GetCount();
        blob.Write( &count );
        for( i32 i = 0; i < count; i++ ) {
            _type_ * res = &resources[ i ];
            TypeDescriptor * type = TypeResolver<_base_ *>::get();
            type->Binary_Write( &res, blob );
        }
    }

    void WindowsCore::EditorOnly_SaveLoadedResourcesToBinary() {
        //BinaryBlob blob = CreateBinaryBlob( Megabytes( 50 ) );
        //const i32 textureCount = resources.textures.GetCount();
        //i32 textureOffset = 0;
        //for( i32 i = 0; i < textureCount; i++ ) {
        //    TextureResource * res = resources.textures.Get( i );
        //    blob.Write( &res->name );
        //    blob.Write( &textureOffset );
        //    textureOffset += res->GetByteSize();
        //}
        //
        //for( i32 i = 0; i < textureCount; i++ ) {
        //    TextureResource * res = resources.textures.Get( i );
        //    ContentTextureProcessor tp = {};
        //    tp.LoadFromFile( res->name.GetCStr() );
        //    i32 size = res->GetByteSize();
        //    blob.Write( &size );
        //    blob.Write( tp.pixelData, size );
        //}

        //const i32 audioCount = resources.audios.GetCount();
        //for( i32 i = 0; i < audioCount; i++ ) {
        //    AudioResource * res = resources.audios.Get( i );
        //
        //}

        //BinaryBlob blob = CreateBinaryBlob( Megabytes( 150 ) );
        //WriteResourceArray<Win32TextureResource, TextureResource>( blob, resources.textures );
        //ResourceWriteEntireBinaryFile( "res/sprites/textures.bin", (char*)blob.buffer, blob.current );
        //
        //blob.current = 0;
        //WriteResourceArray<Win32AudioResource, AudioResource>( blob, resources.audios );
        //ResourceWriteEntireBinaryFile( "res/sprites/audios.bin", (char *)blob.buffer, blob.current );
        //
        //const i32 spriteCount = resources.sprites.GetCount();
        //for( i32 i = 0; i < spriteCount; i++ ) {
        //    SpriteResource res = resources.sprites[ i ];
        //    TypeDescriptor * type = TypeResolver<SpriteResource>::get();
        //    type->Binary_Write( &res, blob );
        //}
    }

#endif

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

