#include "atto_editor.h"
#include "../shared/atto_core.h"
#include "../game/atto_game.h"
#include "../game/modes/atto_game_mode_game.h"
#include "imgui.h"

#include "../win32/atto_core_windows.h"

namespace atto {

    void Editor::UpdateAndRender( Core * core, Game * game, f32 dt ) {
        GameModeGame * leGame = ( (GameModeGame *)game->gameMode );
        if( core->InputKeyJustPressed( KEY_CODE_F1 ) == true ) {
            leGame->localPlayer->camera.noclip = !leGame->localPlayer->camera.noclip;
            core->InputEnableMouse();
        }

        MainMenuBar( core, game );
        Canvas( core, game );

        bool mouseForUI = ImGui::GetIO().WantCaptureMouse;
        if( leGame != nullptr ) {
            leGame->localPlayer->sleeping = mouseForUI;
        }
        game->UpdateAndRender( core, dt );
    }

    void Editor::MainMenuBar( Core * core, Game * game ) {
        GameModeGame * leGame = ( (GameModeGame *)game->gameMode );
        if( ImGui::BeginMainMenuBar() ) {

            if( ImGui::MenuItem( "Canvas" ) ) {
                showCanvas = !showCanvas;
            }

            if( ImGui::MenuItem( "Save Map" ) ) {
                MapFile * mapFile = core->MemoryAllocateTransient<MapFile>();
                leGame->map.SaveToMapFile( mapFile );
               
                TypeDescriptor * mapType = TypeResolver<MapFile>::get();
                nlohmann::json j = mapType->JSON_Write( mapFile );
                std::string json = j.dump( 4 );
                
                WindowsCore::DEBUG_WriteTextFile( "map.json", json.c_str() );
            }

            if( ImGui::MenuItem( "Load Map" ) ) {
                MapFile * mapFile = core->MemoryAllocateTransient<MapFile>();
                TypeDescriptor * mapType = TypeResolver<MapFile>::get();

                char * mapText = (char *)core->MemoryAllocateTransient( Megabytes( 10 ) );
                WindowsCore::DEBUG_ReadTextFile( "map.json", mapText, Megabytes( 10 ) );

                nlohmann::json j = nlohmann::json::parse( mapText );

                mapType->JSON_Read( j, mapFile );

                leGame->map.LoadFromMapFile( mapFile );
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Editor::Canvas( Core * core, Game * game ) {
        if( showCanvas == true ) {
            GameModeGame * leGame = ( (GameModeGame *)game->gameMode );
            Map * map = &leGame->map;
            if( ImGui::Begin( "Canvas", &showCanvas ) ) {
                static ImVec2 scrolling( 0.0f, 0.0f );
                static bool addingLine = false;

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
                    if( core->InputKeyDown( KEY_CODE_LEFT_ALT ) ) {
                        map->RemoveBlock( x, y );
                    }
                    else {
                        map->AddBlock( x, y );
                    }
                    
                    map->Bake();
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
                    if( ImGui::MenuItem( "Remove one", NULL, false ) ) { }
                    if( ImGui::MenuItem( "Remove all", NULL, false ) ) { }
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
                    ImVec2 p1 = ImVec2( origin.x + mousePosSnapped.x, origin.y + mousePosSnapped.y );
                    ImVec2 p2 = ImVec2( p1.x + GRID_STEP, p1.y + GRID_STEP );
                    drawList->AddCircleFilled( p1, 5, IM_COL32( 255, 255, 0, 255 ) );
                    ImU32 c = core->InputKeyDown( KEY_CODE_LEFT_ALT ) ? IM_COL32( 200, 0, 0, 150 ) : IM_COL32( 200, 200, 0, 150 );
                    drawList->AddRectFilled( p1, p2, c );
                    SmallString t = StringFormat::Small( "(%d, %d)", (i32)( mousePosSnapped.x / GRID_STEP ), (i32)( mousePosSnapped.y / GRID_STEP ) );
                    ImVec2 p3 = ImVec2( p1.x + GRID_STEP / 2, p1.y + GRID_STEP / 2);
                    ImVec2 s = ImGui::CalcTextSize( t.GetCStr() );
                    p3.x -= s.x / 2;
                    p3.y -= s.y / 2;
                    drawList->AddText( p3, IM_COL32( 255, 255, 255, 255 ), t.GetCStr() );
                }

                drawList->PopClipRect();
            }

            ImGui::End();
        }
    }
}