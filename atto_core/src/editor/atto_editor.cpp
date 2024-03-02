#include "atto_editor.h"
#include "../shared/atto_core.h"

#include "imgui.h"

namespace atto {

    Editor::Editor() {
        ZeroStructPtr( this );
    }

    void Editor::UpdateAndRender( Core * core, GameMode * game, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( game->IsInitialized() == false ) {
            game->Initialize( core );
            return;
        }

        game->UpdateAndRender( core, dt );

        EngineImgui::NewFrame();
        MainMenuBar( core );
        ResourceEditor( core );
        //ImGui::ShowDemoWindow();
        EngineImgui::EndFrame();
    }

    void Editor::MainMenuBar( Core * core ) {
        if( ImGui::BeginMainMenuBar() ) {
            if( ImGui::MenuItem( "Canvas" ) ) {
                show.canvas = !show.canvas;
            }

            if( ImGui::MenuItem( "Sprites" ) ) {
                show.resourceEditor = !show.resourceEditor;
            }


            if( ImGui::MenuItem( "Save Map" ) ) {
                //leGame->map.DEBUG_SaveToFile( core, "res/maps/map.json" );
            }

            if( ImGui::MenuItem( "Load Map" ) ) {
                //leGame->map.DEBUG_LoadFromFile( core, "res/maps/map.json" );
            }

            ImGui::EndMainMenuBar();
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


    void Editor::ResourceEditor( Core * core ) {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags = ImGuiWindowFlags_MenuBar;

        if( show.resourceEditor && ImGui::Begin( "Resource Editor", &show.resourceEditor, windowFlags ) ) {
            if( ImGui::BeginMenuBar() ) {
                if( ImGui::BeginMenu( "File" ) ) {
                    LargeString res = {};
                    if( ImGui::MenuItem( "New" ) ) {
                        if( core->WindowOpenNativeFolderDialog( nullptr, res ) ) {
                            i32 resIndex = res.FindFirstOf( "res/sprites/" );
                            if( resIndex != -1 ) {
                                res = res.SubStr( resIndex + 12 );
                                resourceWidget.spriteResource = core->ResourceGetAndCreateSprite( res.GetCStr(), 0, 0, 0, 0 );
                            }
                        }
                    }

                    if( ImGui::MenuItem( "Open" ) ) {
                        core->WindowOpenNativeFileDialog( nullptr, res );
                        i32 resIndex = res.FindFirstOf( "res/sprites/" );
                        if( resIndex != -1 ) {
                            res = res.SubStr( resIndex + 12 );
                            res.StripFile();
                            resourceWidget.spriteResource = core->ResourceGetAndLoadSprite( res.GetCStr() );
                        }
                    }
                    
                    if( ImGui::MenuItem( "Save" ) ) {
                        if( resourceWidget.spriteResource != nullptr ) {
                            res = resourceWidget.spriteResource->GetResourcePath();
                            core->ResourceSaveRefl( resourceWidget.spriteResource, res.GetCStr() );
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if( resourceWidget.spriteResource != nullptr ) {
                TypeDescriptor * settingsType = TypeResolver<SpriteResource>::get();
                settingsType->Imgui_Draw( resourceWidget.spriteResource, resourceWidget.spriteResource->spriteName.GetCStr() );
            }

            ImGui::End();
        }
    }

}