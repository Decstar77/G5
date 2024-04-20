#include "../shared/atto_client.h"
#include "../game/atto_game.h"
#include "../shared/atto_colors.h"
#include "../shared/atto_mesh_generation.h"
#include "../content/atto_content.h"

#include "atto_core_windows.h"

#include <fstream>
#include "../editor/atto_editor.h"
#include "../game/modes/atto_game_mode_game.h"
#include "../game/modes/atto_game_mode_main_menu.h"
#include "../game/sim/atto_sim_load_assets.h"

#include "opengl/atto_opengl.h"
#include "vulkan/atto_vulkan.h"

namespace atto {
    void WindowsCore::Run( int argc, char ** argv ) {
        theCore = this;

        OsParseStartArgs( argc, argv );

        Win32SetupCrashReporting();

        MemoryMakePermanent( Megabytes( 128 ) );
        MemoryMakeTransient( Megabytes( 128 ) );

        window.Initialize( this );

        NetworkStart();

        GLStart();
        //VkStart();

        AudioInitialize();

        taskScheduler.Initialize( 4 );

    #if ATTO_EDITOR
        //EngineImgui::Initialize( window.window );
        editor = new Editor();
        editor->Initialize( this );
    #endif

        //game = new GameModeGame();
        currentGameMode = new GameMode_MainMenu();
        currentGameMode->Initialize( this );

        static AudioResource * sovietMarch = ResourceGetAndCreateAudio( "res/sounds/not_legal/redaleart3/soviet_march.mp3", true, false, 0, 0);
        //static AudioResource * sovietMarch = ResourceGetAndCreateAudio( "res/sounds/Shades_of_Defeat.mp3", true, false, 0, 0 );
        static AudioResource * hellMarch = ResourceGetAndCreateAudio( "res/sounds/not_legal/redaleart3/hell_march.mp3", true, false, 0, 0);

        //AudioPlay( sovietMarch, nullptr );

        //LoadAllAssets( this );

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

    void WindowsCore::SetTheCurrentTime( f64 t ) {
        window.SetTime( t );
    }

    void WindowsCore::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
    #if ATTO_OPENGL
        return glState->RenderSubmit( dcxt, clearBackBuffers );
    #elif ATTO_VULKAN
        return vkState->RenderSubmit( dcxt, clearBackBuffers );
    #endif
        
    }

    TextureResource * WindowsCore::ResourceGetAndCreateTexture( const char * name, bool genMips, bool genAnti ) {
    #if ATTO_OPENGL
        return glState->ResourceGetAndCreateTexture( name, genMips, genAnti );
    #endif

        return nullptr;
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

    TextureResource * WindowsCore::ResourceGetAndLoadTexture( const char * name ) {
        return ResourceGetAndCreateTexture( name, false , false );
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
        window.SetCursorDisable();
    }

    void WindowsCore::InputEnableMouse() {
        window.SetCursorNormal();
    }

    bool WindowsCore::InputIsMouseDisabled() {
        return window.IsCursorDisabled();
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


}

