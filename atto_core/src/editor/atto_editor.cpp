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
            core->RenderSetCameraDims( 320, 180 );
            inGame = !inGame;
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_Game( parms ) );
        }
        
        if( core->InputKeyJustPressed( KEY_CODE_F6 ) == true ) {
            GameStartParams parms = {};
            parms.isMutliplayer = false;
            core->MoveToGameMode( new GameMode_Game( parms ) );
        }

        if( game->IsInitialized() == false ) {
            game->Initialize( core );
            return;
        }

        currentMap = game->GetMap();

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

            if( inputHandled == false ) {
                f32 cameraSpeed = 100.0f;
                if( core->InputKeyDown( KeyCode::KEY_CODE_W ) ) {
                    cameraPos.y += cameraSpeed * dt;
                }
                if( core->InputKeyDown( KeyCode::KEY_CODE_S ) ) {
                    cameraPos.y -= cameraSpeed * dt;
                }
                if( core->InputKeyDown( KeyCode::KEY_CODE_A ) ) {
                    cameraPos.x -= cameraSpeed * dt;
                }
                if( core->InputKeyDown( KeyCode::KEY_CODE_D ) ) {
                    cameraPos.x += cameraSpeed * dt;
                }

                f32 zoomDelta = core->InputMouseWheelDelta() * 4.0f;
                cameraWidth -= zoomDelta;
                cameraHeight -= zoomDelta;
                core->RenderSetCameraDims( cameraWidth, cameraHeight );
            }

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

            if( currentMap != nullptr ) {
                const bool clickedThisFrame = core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 );

                EntList & entities = *core->MemoryAllocateTransient<EntList>();
                currentMap->entityPool.GatherActiveObjs( entities );
                const i32 entityCount = entities.GetCount();

                for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                    Entity * ent = entities[ entityIndex ];
                    if( ent->selected ) {
                        editorDrawContext->DrawTexture( gizmoTranslationTexture, ent->pos );

                        BoxBounds2D rightAxisArrowBB = {};
                        rightAxisArrowBB.min = ent->pos + glm::vec2( 2, -5 );
                        rightAxisArrowBB.max = ent->pos + glm::vec2( 16, 5 );

                        BoxBounds2D upAxisArrowBB = {};
                        upAxisArrowBB.min = ent->pos + glm::vec2( -5, 2 );
                        upAxisArrowBB.max = ent->pos + glm::vec2(  5, 16 );

                        BoxBounds2D tranlationBox = {};
                        tranlationBox .min = ent->pos + glm::vec2( 8, 8 );
                        tranlationBox .max = ent->pos + glm::vec2( 16, 16 );

                        if( clickedThisFrame == true && inputHandled == false ) {
                            if( rightAxisArrowBB.Contains( mousePosWorld ) == true ) {
                                gizmoEntity = ent;
                                gizmoDrag = mousePosWorld;
                                gizmoMode = 1;
                            }
                            if( upAxisArrowBB.Contains( mousePosWorld ) == true ) {
                                gizmoEntity = ent;
                                gizmoDrag = mousePosWorld;
                                gizmoMode = 2;
                            }
                            if( tranlationBox.Contains( mousePosWorld ) == true ) {
                                gizmoEntity = ent;
                                gizmoDrag = mousePosWorld;
                                gizmoMode = 3;
                            }
                        }

                        //const glm::vec4 selectionColor( 0.8f, 0.8f, 0.5f, 0.4f );
                        //debugDrawContext->DrawRect( rightAxisArrowBB.min, rightAxisArrowBB.max, selectionColor );
                        //debugDrawContext->DrawRect( upAxisArrowBB.min, upAxisArrowBB.max, selectionColor );
                        //debugDrawContext->DrawRect( tranlationBox.min, tranlationBox.max, selectionColor );
                    }
                }

                if( gizmoEntity != nullptr ) {
                    inputHandled = true;
                    if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                        gizmoEntity = nullptr;
                    }
                    else {
                        glm::vec2 delta = mousePosWorld - gizmoDrag;

                        delta.x = (f32)((i32)delta.x);
                        delta.y = (f32)((i32)delta.y);

                        f32 snapAmount = 16;

                        if( gizmoMode == 1 || gizmoMode == 3 ) {
                            if( glm::abs(delta.x) >= snapAmount ) {
                                gizmoEntity->pos.x += snapAmount * glm::sign( delta.x );
                                gizmoDrag = mousePosWorld;
                            }
                        }

                        if( gizmoMode == 2 || gizmoMode == 3 ) {
                            if( glm::abs( delta.y ) >= snapAmount ) {
                                gizmoEntity->pos.y += snapAmount * glm::sign( delta.y );
                                gizmoDrag = mousePosWorld;
                            }
                        }
                    }
                }

                if( clickedThisFrame == true && inputHandled == false ) {
                    for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                        Entity * ent = entities[ entityIndex ];
                        ent->selected = false;
                        switch( ent->selectionCollider.type ) {
                            case COLLIDER_TYPE_CIRCLE:
                            {
                                Collider2D c = ent->GetWorldSelectionCollider();
                                if( c.Contains( mousePosWorld ) == true ) {
                                    ent->selected = true;
                                }
                            } break;
                            case COLLIDER_TYPE_BOX:
                            {
                                Collider2D b = ent->GetWorldSelectionCollider();
                                if( b.Contains( mousePosWorld ) == true ) {
                                    ent->selected = true;
                                }
                            } break;
                        }
                    }
                }
            }

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
        ImGuiWindowFlags windowFlags = 0;
        if( show.canvas && ImGui::Begin( "Canvas", &show.canvas, windowFlags ) ) {
            TileMap & tileMap = currentMap->tileMap;
            SpriteResourcePicker( core, tileMap.spriteResource );

            ImGui::InputInt( "X Count", &tileMap.tileXCount );
            ImGui::InputInt( "Y Count", &tileMap.tileYCount );
            ImGui::Checkbox( "Flags", &canvasWidget.showFlags );

            const i32 gridSize = 4; // Change this value to adjust the grid size
            TileMapTile * tile = nullptr;
            DrawContext * editorDrawContext = core->RenderGetDrawContext( 3, false );

            if( tileMap.spriteResource != nullptr && tileMap.spriteResource->textureResource != nullptr ) {

                if( canvasWidget.showFlags == false ) {
                    const i32 xCount = (i32)( (f32)tileMap.spriteResource->textureResource->width / TILE_SIZE );
                    const i32 yCount = (i32)( (f32)tileMap.spriteResource->textureResource->height / TILE_SIZE );
                    const i32 count = xCount * yCount;
                    for( i32 tileIndex = 0; tileIndex < count; tileIndex++ ) {
                        const i32 tileX = tileIndex % gridSize;
                        const i32 tileY = tileIndex / gridSize;

                        glm::vec2 uvMin = glm::vec2( 0 );
                        glm::vec2 uvMax = glm::vec2( 1 );
                        tileMap.spriteResource->GetUVForTile( tileX, tileY, uvMin, uvMax );

                        ImVec2 size = ImVec2( TILE_SIZE, TILE_SIZE );
                        ImVec2 uvMin_ = ImVec2( uvMin.x, uvMin.y );
                        ImVec2 uvMax_ = ImVec2( uvMax.x, uvMax.y );
                        ImGui::PushID( tileIndex );
                        if( ImGui::ImageButton( (void *)(intptr_t)tileMap.spriteResource->textureResource->handle, size, uvMin_, uvMax_ ) ) {
                            canvasWidget.seletedTileIndex = tileIndex;
                        }
                        ImGui::PopID();
                        if( tileIndex % gridSize < gridSize - 1 ) {
                            ImGui::SameLine();
                        }
                    }
                }
                else {
                    canvasWidget.flagValues = 0;
                    canvasWidget.seletedTileIndex = -1;

                    static bool noWalkFlag = false;
                    ImGui::Checkbox( Stringify( TILE_FLAG_NO_WALK ), &noWalkFlag );

                    if( noWalkFlag == true ) {
                        canvasWidget.flagValues |= TILE_FLAG_NO_WALK;
                    }
                }

                // Draw grid lines
                const i32 tileSize = (i32)TILE_SIZE;
                const i32 gridCountX = tileMap.tileXCount;
                const i32 gridCountY = tileMap.tileYCount;
                const glm::vec2 gridStart = glm::vec2( 0 );
                const glm::vec2 gridEnd = glm::vec2( gridCountX * tileSize, gridCountY * tileSize );
                for( i32 x = 0; x < gridCountX; x++ ) {
                    const glm::vec2 start = glm::vec2( x * tileSize, 0 );
                    const glm::vec2 end = glm::vec2( x * tileSize, gridCountY * tileSize );
                    editorDrawContext->DrawLine2D( start, end, 1, Colors::CARROT );
                }

                for( i32 y = 0; y < gridCountY; y++ ) {
                    const glm::vec2 start = glm::vec2( 0, y * tileSize );
                    const glm::vec2 end = glm::vec2( gridCountX * tileSize, y * tileSize );
                    editorDrawContext->DrawLine2D( start, end, 1, Colors::CARROT );
                }

                if( canvasWidget.showFlags == true ) {
                    for( i32 y = 0; y < gridCountY; y++ ) {
                        for( i32 x = 0; x < gridCountX; x++ ) {
                            tile = tileMap.GetTile( x, y );
                            if( tile != nullptr ) {
                                const i32 flags = tile->flags;
                                if( EnumHasFlag( flags, TILE_FLAG_NO_WALK ) ) {
                                    editorDrawContext->DrawRect( tile->center, glm::vec2( TILE_SIZE ), 0.0f, glm::vec4( 0.8f, 0.2f, 0.2f, 0.8f ) );
                                }
                            }
                        }
                    }
                }
                
            }

            ImGui::End();

            if( ImGui::GetIO().WantCaptureMouse == false ) {
                const glm::vec2 mousePosPix = core->InputMousePosPixels();
                const glm::vec2 mousePosWorld = editorDrawContext->ScreenPosToWorldPos( mousePosPix );
                if( canvasWidget.seletedTileIndex != -1 ) {
                    const i32 tileX = canvasWidget.seletedTileIndex % gridSize;
                    const i32 tileY = canvasWidget.seletedTileIndex / gridSize;
                    tile = tileMap.GetTile( mousePosWorld );
                    if( tile != nullptr ) {
                        editorDrawContext->DrawSprite( tileMap.spriteResource, tileX, tileY, tile->center );
                        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                            currentMap->Editor_MapTilePlace( tile->xIndex, tile->yIndex, tileX, tileY, 0 );
                        }
                    }
                }

                if( canvasWidget.showFlags == true ) {
                    tile = tileMap.GetTile( mousePosWorld );
                    if( tile != nullptr ) {
                        editorDrawContext->DrawRect( tile->center, glm::vec2( TILE_SIZE ), 0.0f, glm::vec4( 0.8f, 0.2f, 0.2f, 0.8f ) );
                        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                            tile->flags = canvasWidget.flagValues;
                        }
                    }
                }

            }
        }
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
                                resourceWidget.entity = currentMap->SpawnEntitySim( core, EntityType::Make( EntityType::ENEMY_BOT_TURRET ), glm::vec2( 0 ), glm::vec2( 0 ) );
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
                TypeDescriptor * settingsType = TypeResolver<Entity>::get();
                settingsType->Imgui_Draw( resourceWidget.entity, "Entity" );
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
