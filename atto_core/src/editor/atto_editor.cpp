#include "atto_editor.h"
#include "../shared/atto_core.h"

#include "imgui.h"

namespace atto {

    Editor::Editor() {
        editorCamera.pos = glm::vec3( 0.0f, 0.0f, 3.0f );
        editorCamera.camera = EntCamera::CreateDefault();
    }

    void Editor::UpdateAndRender( Core * core, GameMode * game, f32 dt ) {
        if( core->InputKeyDown( KEY_CODE_ESCAPE ) ) {
            core->WindowClose();
        }

        if( game->IsInitialized() == false ) {
            game->Initialize( core );
            Map & map = ( (GameModeGame *)game )->map;
            map.DEBUG_LoadFromFile( core, "res/maps/map.json" );
            map.Start( core );
        }

        if( core->InputKeyJustPressed( KEY_CODE_F6 ) == true ) {
            core->WindowSetVSync( !core->WindowGetVSync() );
        }

        GameModeGame * leGame = ( (GameModeGame *)game );
        if( core->InputKeyJustPressed( KEY_CODE_F1 ) == true ) {
            editorActive = !editorActive;
            if( editorActive == false ) {
                for( i32 i = 0; i < ArrayCount( windowShows ); i++ ) {
                    windowShows[ i ] = false;
                }

                EnumRemoveFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_NO_UPDATE );
                EnumRemoveFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER );
            }
            else {
                EnumSetFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_NO_UPDATE );
                EnumSetFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER );
                core->InputEnableMouse();
            }
        }

        if( editorActive == true ) {
            MainMenuBar( core, game );
            Canvas( core, game );
            bool mouseForUI = ImGui::GetIO().WantCaptureMouse;
            if( mouseForUI == false ) {
                UpdateEditorCamera( core );
            }
        }

        game->UpdateAndRender( core, dt, (UpdateAndRenderFlags)updateAndRenderFlags );

        if( editorActive == true ) {
            DrawContext * worldDraws = core->RenderGetDrawContext( 0, false );
            worldDraws->SetCamera( editorCamera.GetViewMatrix(), editorCamera.camera.yfov, editorCamera.camera.zNear, editorCamera.camera.zFar );
            core->RenderSubmit( worldDraws, true );
        }
    }

    void Editor::MainMenuBar( Core * core, GameMode * game ) {
        GameModeGame * leGame = ( (GameModeGame *)game );
        if( ImGui::BeginMainMenuBar() ) {

            if( ImGui::MenuItem( "Canvas" ) ) {
                show.canvas = !show.canvas;
            }

            if( ImGui::MenuItem( "Save Map" ) ) {
                leGame->map.DEBUG_SaveToFile( core, "res/maps/map.json" );
            }

            if( ImGui::MenuItem( "Load Map" ) ) {
                leGame->map.DEBUG_LoadFromFile( core, "res/maps/map.json" );
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Editor::Canvas( Core * core, GameMode * game ) {
        enum BrushType {
            BRUSH_TYPE_BLOCK,
            BRUSH_TYPE_PLAYER_START,
            BRUSH_TYPE_COUNT,
        };

        if( show.canvas == true ) {
            GameModeGame * leGame = ( (GameModeGame *)game );
            Map * map = &leGame->map;
            if( ImGui::Begin( "Canvas", &show.canvas ) ) {
                static ImVec2 scrolling( 0.0f, 0.0f );
                static bool addingLine = false;
                static BrushType brushType = BRUSH_TYPE_BLOCK;

            #define WorldMapToGrid(p) ImVec2( origin.x + p.x * GRID_STEP, origin.y + p.z * GRID_STEP )
            #define WorldMapToGridTileCenter(p) ImVec2( origin.x + p.x * GRID_STEP + GRID_STEP / 2.0f, origin.y + p.z * GRID_STEP + GRID_STEP / 2.0f )

                const char * brushTypeNames[ BRUSH_TYPE_COUNT ] = {
                    "Block",
                    "Player Start",
                };
                ImGui::Combo( "Brush Type", (int *)&brushType, brushTypeNames, BRUSH_TYPE_COUNT );

                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
                if( canvas_sz.x < 50.0f ) { canvas_sz.x = 50.0f; }
                if( canvas_sz.y < 50.0f ) { canvas_sz.y = 50.0f; }
                ImVec2 canvas_p1 = ImVec2( canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y );

                // Draw border and background color
                ImGuiIO & io = ImGui::GetIO();
                ImDrawList * drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled( canvas_p0, canvas_p1, IM_COL32( 50, 50, 50, 255 ) );
                drawList->AddRect( canvas_p0, canvas_p1, IM_COL32( 255, 255, 255, 255 ) );

                // This will catch our interactions
                ImGui::InvisibleButton( "canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight );
                const bool is_hovered = ImGui::IsItemHovered(); // Hovered
                const bool is_active = ImGui::IsItemActive();   // Held
                const ImVec2 origin( canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y ); // Lock scrolled origin
                const ImVec2 mousePosInCanvas( io.MousePos.x - origin.x, io.MousePos.y - origin.y );
                const float GRID_STEP = 64.0f;
                ImVec2 mousePosSnapped = {};
                mousePosSnapped.x = i32( mousePosInCanvas.x / GRID_STEP ) * GRID_STEP;
                mousePosSnapped.y = i32( mousePosInCanvas.y / GRID_STEP ) * GRID_STEP;

                if( is_hovered && core->InputMouseButtonDown( MOUSE_BUTTON_1 ) ) {
                    i32 x = (i32)( mousePosSnapped.x / GRID_STEP );
                    i32 y = (i32)( mousePosSnapped.y / GRID_STEP );

                    switch( brushType ) {
                        case BRUSH_TYPE_BLOCK:
                        {
                            if( core->InputKeyDown( KEY_CODE_LEFT_ALT ) ) {
                                map->RemoveBlock( x, y );
                            }
                            else {
                                map->AddBlock( x, y );
                            }
                            map->Bake();
                        } break;

                        case BRUSH_TYPE_PLAYER_START:
                        {
                            map->playerStartPos = glm::vec3( x, 0.0f, y );
                        } break;

                        default:
                        {
                            INVALID_CODE_PATH;
                        } break;
                    }
                }

                // Pan (we use a zero mouse threshold when there's no context menu)
                // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
                const float mouse_threshold_for_pan = true ? -1.0f : 0.0f;
                if( is_active && ImGui::IsMouseDragging( ImGuiMouseButton_Right, mouse_threshold_for_pan ) ) {
                    scrolling.x += io.MouseDelta.x;
                    scrolling.y += io.MouseDelta.y;
                }

                // Context menu (under default mouse threshold)
                ImVec2 drag_delta = ImGui::GetMouseDragDelta( ImGuiMouseButton_Right );
                if( drag_delta.x == 0.0f && drag_delta.y == 0.0f )
                    ImGui::OpenPopupOnItemClick( "context", ImGuiPopupFlags_MouseButtonRight );
                if( ImGui::BeginPopup( "context" ) ) {
                    addingLine = false;
                    if( ImGui::MenuItem( "Remove one", NULL, false ) ) {}
                    if( ImGui::MenuItem( "Remove all", NULL, false ) ) {}
                    ImGui::EndPopup();
                }

                // Draw grid + all lines in the canvas
                drawList->PushClipRect( canvas_p0, canvas_p1, true );
                if( true ) {
                    for( float x = fmodf( scrolling.x, GRID_STEP ); x < canvas_sz.x; x += GRID_STEP ) {
                        drawList->AddLine( ImVec2( canvas_p0.x + x, canvas_p0.y ), ImVec2( canvas_p0.x + x, canvas_p1.y ), IM_COL32( 200, 200, 200, 40 ) );
                    }
                    for( float y = fmodf( scrolling.y, GRID_STEP ); y < canvas_sz.y; y += GRID_STEP ) {
                        drawList->AddLine( ImVec2( canvas_p0.x, canvas_p0.y + y ), ImVec2( canvas_p1.x, canvas_p0.y + y ), IM_COL32( 200, 200, 200, 40 ) );
                    }
                }

                const i32 mapBlockCount = map->blocks.GetCapcity();
                for( i32 blockIndex = 0; blockIndex < mapBlockCount; blockIndex++ ) {
                    const MapBlock & b = map->blocks[ blockIndex ];
                    if( b.filled ) {
                        ImVec2 minV = { origin.x + b.xIndex * GRID_STEP,            origin.y + b.yIndex * GRID_STEP };
                        ImVec2 maxV = { origin.x + ( b.xIndex + 1 ) * GRID_STEP,    origin.y + ( b.yIndex + 1 ) * GRID_STEP };

                        drawList->AddRectFilled( minV, maxV, IM_COL32( 230, 230, 0, 255 ) );
                    }
                }

                {
                    ImVec2 p1 = WorldMapToGridTileCenter( map->playerStartPos );
                    drawList->AddCircleFilled( p1, 15, IM_COL32( 20, 200, 200, 255 ) );
                }


                {
                    ImVec2 p1 = ImVec2( origin.x + mousePosSnapped.x, origin.y + mousePosSnapped.y );
                    ImVec2 p2 = ImVec2( p1.x + GRID_STEP, p1.y + GRID_STEP );
                    ImVec2 p3 = ImVec2( p1.x + GRID_STEP / 2, p1.y + GRID_STEP / 2 );
                    drawList->AddCircleFilled( p1, 5, IM_COL32( 255, 255, 0, 255 ) );

                    switch( brushType ) {
                        case BRUSH_TYPE_BLOCK:
                        {
                            ImU32 c = core->InputKeyDown( KEY_CODE_LEFT_ALT ) ? IM_COL32( 200, 0, 0, 150 ) : IM_COL32( 200, 200, 0, 150 );
                            drawList->AddRectFilled( p1, p2, c );
                        } break;

                        case BRUSH_TYPE_PLAYER_START:
                        {
                            drawList->AddCircleFilled( p3, 15, IM_COL32( 20, 200, 200, 150 ) );
                        } break;

                        default:
                        {
                            INVALID_CODE_PATH;
                        } break;
                    }

                  

                    SmallString t = StringFormat::Small( "(%d, %d)", (i32)( mousePosSnapped.x / GRID_STEP ), (i32)( mousePosSnapped.y / GRID_STEP ) );
                    ImVec2 s = ImGui::CalcTextSize( t.GetCStr() );
                    p3.x -= s.x / 2;
                    p3.y -= s.y / 2;
                    drawList->AddText( p3, IM_COL32( 255, 255, 255, 255 ), t.GetCStr() );
                }

                drawList->PopClipRect();

            #undef WorldMapToGrid
            #undef WorldMapToGridTileCenter
            }

            ImGui::End();
        }
    }

    void Editor::UpdateEditorCamera( Core * core ) {
        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
            core->InputDisableMouse();
            return;
        }

        if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
            core->InputEnableMouse();
            return;
        }

        if( core->InputMouseButtonDown( MOUSE_BUTTON_2 ) == true ) {
            f32 v = editorCamera.camera.movementSpeed * core->GetDeltaTime();
            if( core->InputKeyDown( KEY_CODE_W ) ) {
                editorCamera.pos += editorCamera.camera.front * v;
            }
            if( core->InputKeyDown( KEY_CODE_S ) ) {
                editorCamera.pos -= editorCamera.camera.front * v;
            }
            if( core->InputKeyDown( KEY_CODE_A ) ) {
                editorCamera.pos -= editorCamera.camera.right * v;
            }
            if( core->InputKeyDown( KEY_CODE_D ) ) {
                editorCamera.pos += editorCamera.camera.right * v;
            }
            if( core->InputKeyDown( KEY_CODE_SPACE ) ) {
                editorCamera.pos.y += v;
            }
            if( core->InputKeyDown( KEY_CODE_LEFT_CONTROL ) ) {
                editorCamera.pos.y -= v;
            }

            glm::vec2 mouseDelta = core->InputMouseDeltaPixels();
            f32 xoffset = mouseDelta.x * editorCamera.camera.mouseSensitivity;
            f32 yoffset = mouseDelta.y * editorCamera.camera.mouseSensitivity;
            editorCamera.camera.yaw += xoffset;
            editorCamera.camera.pitch += yoffset;
            if( editorCamera.camera.pitch > 89.0f ) {
                editorCamera.camera.pitch = 89.0f;
            }
            if( editorCamera.camera.pitch < -89.0f ) {
                editorCamera.camera.pitch = -89.0f;
            }

            glm::vec3 front;
            front.x = cos( glm::radians( editorCamera.camera.yaw ) ) * cos( glm::radians( editorCamera.camera.pitch ) );
            front.y = sin( glm::radians( editorCamera.camera.pitch ) );
            front.z = sin( glm::radians( editorCamera.camera.yaw ) ) * cos( glm::radians( editorCamera.camera.pitch ) );
            editorCamera.camera.front = glm::normalize( front );
            editorCamera.camera.right = glm::normalize( glm::cross( editorCamera.camera.front, glm::vec3( 0, 1, 0 ) ) );
            editorCamera.camera.up = glm::normalize( glm::cross( editorCamera.camera.right, editorCamera.camera.front ) );
        }
    }

}