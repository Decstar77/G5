#include "../../game/atto_game.h"
#include "../../shared/atto_colors.h"

#include "../../content/atto_content.h"

#include "atto_win32_core.h"

#include <fstream>
#include "../../game/modes/atto_game_mode_game.h"
#include "../../game/modes/atto_game_mode_main_menu.h"

#include "opengl/atto_opengl.h"
#include "vulkan/atto_vulkan.h"

namespace atto {
    void WindowsCore::Run( int argc, char ** argv ) {
        theCore = this;
        OsParseStartArgs( argc, argv );
        Win32SetupCrashReporting();
        GlobalArenasMake( Megabytes( 128 ), Megabytes( 128 ) );

        window.Initialize( this );
        GLStart();
        NuklearUIInitialize();
        NetworkStart();
        //VkStart();
        AudioInitialize();
        taskScheduler.Initialize( 4 );

        LoadAllAssets();

        //game = new GameModeGame();
        currentGameMode = new GameMode_MainMenu();
        currentGameMode->Initialize( this );

        //static AudioResource * sovietMarch = ResourceGetAndCreateAudio( "res/sounds/not_legal/redaleart3/soviet_march.mp3", true, false, 0, 0);
        //static AudioResource * sovietMarch = ResourceGetAndCreateAudio( "res/sounds/Shades_of_Defeat.mp3", true, false, 0, 0 );
        //static AudioResource * hellMarch = ResourceGetAndCreateAudio( "res/sounds/not_legal/redaleart3/hell_march.mp3", true, false, 0, 0);
        //AudioPlay( sovietMarch, nullptr );

        this->deltaTime = 0;
        f64 startTime = window.GetTime();
        while( window.ShouldClose() == false ) {
            FrameInput & fi = InputGetFrameInput();
            fi.lastKeys = fi.keys;
            fi.lastMouseButtons = fi.mouseButtons;
            fi.mouseWheelDelta = glm::vec2( 0.0f, 0.0f );
            fi.mouseDeltaPixels = glm::vec2( 0.0f, 0.0f );

            //std::cout << "Poll start " << std::endl;
            window.PollEvents();
            //std::cout << "Poll end " << std::endl;

            if( InputKeyJustPressed( KEY_CODE_ESCAPE ) ) {
                window.Close();
            }

            if( InputKeyJustPressed( KEY_CODE_ENTER ) && InputKeyDown( KEY_CODE_LEFT_ALT ) ) {
                if( window.windowFullscreen ) {
                    window.DisableFullscreen();
                }
                else {
                    window.EnableFullscreen();
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

            if( skipFrame == false ) {
                if( currentGameMode->IsInitialized() == false ) {
                    currentGameMode->Initialize( this );
                    return;
                }

                currentGameMode->UpdateAndRender( this, this->deltaTime );
            }

            AudioUpdate();

            window.SwapBuffers();

            MemoryClearTransient();

            f64 endTime = window.GetTime();
            this->deltaTime = (f32)( endTime - startTime );
            startTime = endTime;
        }
        
    #if ATTO_EDITOR
        //EngineImgui::Shutdown();
    #endif
    }

    f64 WindowsCore::GetTheCurrentTime() const {
        return window.GetTime();
    }

    void WindowsCore::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
    #if ATTO_OPENGL
        return glState->RenderSubmit( dcxt, clearBackBuffers );
    #elif ATTO_VULKAN
        return vkState->RenderSubmit( dcxt, clearBackBuffers );
    #endif
        
    }

    FontHandle WindowsCore::ResourceGetFont( const char * name ) {
    #if ATTO_OPENGL
        return glState->ResourceGetFont( name );
    #endif
        return {};
    }


    float WindowsCore::FontGetTextBounds( FontHandle font, f32 fontSize, const char * text, glm::vec2 pos, BoxBounds2D & bounds, TextAlignment_H hA, TextAlignment_V vA ) {
    #if ATTO_OPENGL
        return glState->FontGetTextBounds( font, fontSize, text, pos, bounds, hA, vA );
    #endif
        return {};
    }

    void WindowsCore::InputDisableMouse() {
        window.SetCursorDisable();
    }

    void WindowsCore::InputEnableMouse() {
        window.SetCursorNormal();
    }

    bool WindowsCore::InputIsMouseDisabled() {
        return window.IsCursorDisabled();
    }

    void * WindowsCore::WindowGetHandle() {
        return window.GetWindowPtr();
    }

    void WindowsCore::WindowClose() {
        window.Close();
    }

    void WindowsCore::WindowSetTitle( const char * title ) {
        window.SetWindowTitle( title );
    }

    void WindowsCore::WindowSetVSync( bool value ) {
        window.SetVysnc( value );
        theGameSettings.vsync = value;
    }

    bool WindowsCore::WindowGetVSync() {
        return theGameSettings.vsync;
    }

    bool WindowsCore::WindowIsFullscreen() {
        return window.windowFullscreen;
    }

    void WindowsCore::OsParseStartArgs( int argc, char ** argv ) {
       
        if( argc > 1 ) {
            ATTOINFO( "Parsing start up settings" );

            // TODO(DECLAN): Don't do this...
            static char buffer[ 2048 ] = {};
            PlatformReadEntireTextFile( argv[ 1 ], buffer, sizeof( buffer ) );
            
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
}

