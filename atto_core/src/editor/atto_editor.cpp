#include "atto_editor.h"
#include "../shared/atto_core.h"
#include "../shared/atto_colors.h"


#include "imgui.h"

#if ATTO_EDITOR

namespace atto {

    Editor::Editor() {
        ZeroStruct( windowShows );
    }

    void Editor::Initialize( Core * core ) {
        gizmoTranslationTexture = core->ResourceGetAndLoadTexture( "res/editor/translation_gizmo.png" );
    }

    void Editor::UpdateAndRender( Core * core, GameMode * game, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( core->InputKeyJustPressed( KEY_CODE_F5 ) == true ) {
            inGame = !inGame;
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_SinglePlayerGame( parms ) );
        }
        
        if( core->InputKeyJustPressed( KEY_CODE_F6 ) == true ) {
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_SinglePlayerGame( parms ) );
        }

        if( game->IsInitialized() == false ) {
            game->Initialize( core );
            return;
        }

        currentMap = nullptr;

        u32 flags = {};
        if( inGame == false ) {
            flags |= UPDATE_AND_RENDER_FLAG_NO_UPDATE;
            flags |= UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER;
        }

        game->UpdateAndRender( core, dt, (UpdateAndRenderFlags)flags );

        if( inGame == false ) {
            DrawContext * editorDrawContext = core->RenderGetDrawContext( 3, true );
            editorDrawContext->SetCameraPos( cameraPos );

            EngineImgui::NewFrame();
            MainMenuBar( core );
            ResourceEditor( core, game );
            CanvasEditor( core, game );

            bool inputHandled = ImGui::GetIO().WantCaptureMouse;

            // Get the draw contexts that the game filled
            DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, false );
            DrawContext * uiDrawContext = core->RenderGetDrawContext( 1, false );
            DrawContext * debugDrawContext = core->RenderGetDrawContext( 2, false );

            const glm::vec2 mousePosPix = core->InputMousePosPixels();
            glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

            if( core->InputMouseButtonDown( MOUSE_BUTTON_2 ) == true ) {
                if( isCameraDragging == true ) {
                    glm::vec2 delta = mousePosWorld - dragStartPos;
                    cameraPos -= delta;
                    dragStartPos = mousePosWorld;
                }
                else {
                    isCameraDragging = true;
                    dragStartPos = mousePosWorld;
                }
            }
            if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
                isCameraDragging = false;
            }

            spriteDrawContext->SetCameraPos( cameraPos );
            debugDrawContext->SetCameraPos( cameraPos );

            mousePosWorld = editorDrawContext->ScreenPosToWorldPos( mousePosPix );

            core->RenderSubmit( spriteDrawContext, true );
            core->RenderSubmit( uiDrawContext, false );
            core->RenderSubmit( debugDrawContext, false );
            core->RenderSubmit( editorDrawContext, false );
           
            //ImGui::ShowDemoWindow();
            EngineImgui::EndFrame();
        }
    }

    void Editor::MainMenuBar( Core * core ) {
        if( ImGui::BeginMainMenuBar() ) {
            if( ImGui::MenuItem( "Resources" ) ) {
                show.resourceEditor = !show.resourceEditor;
            }

            if( ImGui::MenuItem( "Load Map" ) ) {
                //leGame->map.DEBUG_LoadFromFile( core, "res/maps/map.json" );
            }

            if( currentMap != nullptr && inGame == false ) {
                if( ImGui::MenuItem( "Save Map" ) ) {
                    //core->ResourceWriteTextRefl( currentMap, currentMap->mapName.GetCStr() );
                }
            }

            if( currentMap != nullptr && ImGui::MenuItem( "Canvas" ) ) {
                show.canvas = !show.canvas;
            }

            ImGui::EndMainMenuBar();
        }
    }

    inline static bool SpriteResourcePicker( Core * core, SpriteResource *& spriteResource ) {
        LargeString res = {};
        if( spriteResource != nullptr && spriteResource->textureResource != nullptr ) {
            if( ImGui::ImageButton( (void *)(intptr_t)spriteResource->textureResource->handle, ImVec2( 64.0f, 64.0f ) ) ) {
                if( core->WindowOpenNativeFileDialog( nullptr, "png", res ) ) {
                    res.StripFileExtension();
                    res.Add( ".json" );
                    spriteResource = core->ResourceGetAndLoadSprite( res.GetCStr() );
                    return true;
                }
            }
        }
        else {
            if( ImGui::Button( "Select Sprite" ) ) {
                if( core->WindowOpenNativeFileDialog( nullptr, "png", res ) ) {
                    res.StripFileExtension();
                    res.Add( ".json" );
                    spriteResource = core->ResourceGetAndLoadSprite( res.GetCStr() );
                    return true;
                }
            }
        }
        return false;
    }

    void Editor::CanvasEditor( Core * core, GameMode * gameMode ) {
      
    }

    void Editor::ResourceEditor( Core * core, GameMode * gameMode ) {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags = ImGuiWindowFlags_MenuBar;

        if( show.resourceEditor && ImGui::Begin( "Resource Editor", &show.resourceEditor, windowFlags ) ) {
            LargeString res = {};
            if( ImGui::BeginMenuBar() ) {
                if( ImGui::BeginMenu( "File" ) ) {
                    if( ImGui::BeginMenu( "New" ) ) {
                        if( ImGui::MenuItem( "Entity" ) ) {
                            if( currentMap != nullptr ) {
                                //resourceWidget.entity = currentMap->SpawnEntitySim( core, EntityType::Make( EntityType::ENEMY_BOT_TURRET ), glm::vec2( 0 ), glm::vec2( 0 ) );
                                resourceWidget.spriteResource = nullptr;
                            }
                        }
                        if( ImGui::MenuItem( "Sprite" ) ) {
                            if( core->WindowOpenNativeFileDialog( nullptr, "png", res ) ) {
                                res.StripFileExtension();
                                res.Add( ".json" );
                                resourceWidget.spriteResource = core->ResourceGetAndCreateSprite( res.GetCStr(), 1, 0, 0, 0 );
                                resourceWidget.entity = nullptr;
                            }
                        }
                        ImGui::EndMenu();
                    }

                    if( ImGui::MenuItem( "Open" ) ) {
                        if( core->WindowOpenNativeFileDialog( nullptr, nullptr, res ) ) {
                            resourceWidget.spriteResource = core->ResourceGetAndLoadSprite( res.GetCStr() );
                        }
                    }

                    if( ImGui::MenuItem( "Save" ) ) {
                        if( resourceWidget.spriteResource != nullptr ) {
                            res = resourceWidget.spriteResource->GetResourcePath();
                            core->ResourceWriteTextRefl( resourceWidget.spriteResource, res.GetCStr() );
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if( ImGui::Button( "Save" ) == true ) {
                if( resourceWidget.spriteResource != nullptr ) {
                    res = resourceWidget.spriteResource->GetResourcePath();
                    core->ResourceWriteTextRefl( resourceWidget.spriteResource, res.GetCStr() );
                }
            }

            if( resourceWidget.spriteResource != nullptr ) {
                TypeDescriptor * settingsType = TypeResolver<SpriteResource>::get();
                settingsType->Imgui_Draw( resourceWidget.spriteResource, resourceWidget.spriteResource->spriteName.GetCStr() );
            }
            else if( resourceWidget.entity != nullptr ) {
                //TypeDescriptor * settingsType = TypeResolver<Entity>::get();
                //settingsType->Imgui_Draw( resourceWidget.entity, "Entity" );
            }

            ImGui::End();
        }
    }
}

    //struct TestRefl2 {
    //    i32 rr;

    //    REFLECT();
    //};

    //REFLECT_STRUCT_BEGIN( TestRefl2 )
    //    REFLECT_STRUCT_MEMBER( rr )
    //    REFLECT_STRUCT_END()


    //struct TestRefl {
    //    i32 age;
    //    f32 eeee;
    //    SmallString somethign;
    //    TestRefl2 t2;
    //    FixedList<i32, 32> li;

    //    REFLECT();
    //};

    //REFLECT_STRUCT_BEGIN( TestRefl )
    //    REFLECT_STRUCT_MEMBER( age )
    //    REFLECT_STRUCT_MEMBER( eeee )
    //    REFLECT_STRUCT_MEMBER( somethign )
    //    REFLECT_STRUCT_MEMBER( t2 )
    //    REFLECT_STRUCT_MEMBER( li )
    //    REFLECT_STRUCT_END()


    //REFL_ENUM( SomethingNew, 
    //           INVALID, 
    //           OOON, 
    //           TWOOO, 
    //           EEEE 
    //);

    //struct Teester {
    //    SomethingNew s;
    //    REFLECT();
    //};

    //    REFLECT_STRUCT_BEGIN( Teester )
    //        REFLECT_STRUCT_MEMBER( s )
    //        REFLECT_STRUCT_END()


#endif
