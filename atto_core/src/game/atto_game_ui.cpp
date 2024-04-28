#include "atto_game_ui.h"
#include "atto_core.h"
#include "../shared/atto_colors.h"
#include "modes/atto_game_mode_game.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "demo/common/overview.c"

namespace atto {
    typedef struct nk_colorf NkColorf;
    typedef struct nk_glfw NkGlfw;
    typedef struct nk_context NkContext;
    typedef struct nk_font_atlas NkFontAtlas;

    static NkGlfw nkGlfw = { 0 };
    static NkContext * ctx = nullptr;
    static NkFontAtlas * atlas = nullptr;

    void Core::NuklearUIScroll( f64 xoff, f64 yoff ) {
        (void)xoff;
        nkGlfw.scroll.x += (float)xoff;
        nkGlfw.scroll.y += (float)yoff;
    }

    void Core::NuklearUIChar( u32 codepoint ) {
        if( nkGlfw.text_len < NK_GLFW_TEXT_MAX ) {
            nkGlfw.text[ nkGlfw.text_len++ ] = codepoint;
        }
    }

    void Core::NuklearUIMouseButton( int button, int action, int mods ) {
        double x, y;
        NK_UNUSED( mods );
        if( button != GLFW_MOUSE_BUTTON_LEFT ) return;
        glfwGetCursorPos( nkGlfw.win, &x, &y );
        if( action == GLFW_PRESS ) {
            double dt = glfwGetTime() - nkGlfw.last_button_click;
            if( dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI ) {
                nkGlfw.is_double_click_down = nk_true;
                nkGlfw.double_click_pos = nk_vec2( (float)x, (float)y );
            }
            nkGlfw.last_button_click = glfwGetTime();
        }
        else nkGlfw.is_double_click_down = nk_false;
    }

    void Core::NuklearUIInitialize() {
        ctx = nk_glfw3_init( &nkGlfw, (GLFWwindow *)WindowGetHandle(), NK_GLFW3_DEFAULT );
        
        nk_glfw3_font_stash_begin( &nkGlfw, &atlas );
        struct nk_font *kenvector= nk_font_atlas_add_from_file(atlas, "res/game/fonts/kenvector_future.ttf", 20, 0);
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
        nk_glfw3_font_stash_end( &nkGlfw );
        /*nk_style_load_all_cursors(ctx, atlas->curors);*/
        nk_style_set_font(ctx, &kenvector->handle);
    }

    void UI_MainMenu( Core * core ) {
        nk_glfw3_new_frame( &nkGlfw );

        NkColorf bg;
        bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

        ctx->style.window.fixed_background.data.color = { 67, 90, 107, 255 };
        ctx->style.window.border_color = { 123, 147, 157, 255 };

        ctx->style.button.normal.data = { 67, 100, 117, 255 };
        ctx->style.button.border_color = { 101, 158, 177, 255 };
        ctx->style.button.hover.data.color = { 101 - 20, 158 - 20, 177 - 20, 255 };

        f32 surfaceWidth = core->RenderGetMainSurfaceWidth();
        f32 surfaceHeight = core->RenderGetMainSurfaceHeight();

        f32 startX = surfaceWidth * 0.04f;
        f32 startY = surfaceHeight * 0.051f;
        f32 winWidth = surfaceWidth / 5.5f; // 230 roughly
        f32 winHeight = surfaceHeight / 2.88f;

        if( nk_begin( ctx, "MainMenu", nk_rect( startX, startY, winWidth, winHeight ), NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR ) ) {
            nk_layout_row_dynamic( ctx, 30, 1 );
            nk_label( ctx, "The Game Title", NK_TEXT_CENTERED );
        }
        nk_end( ctx );
        
        static AudioResource * sndClick = ResourceGetAndCreateAudio( "res/game/gui/main_menu_click.wav",{ true, false, 0, 0 } );
        sndClick->maxInstances = 10;
        sndClick->stealMode = AudioStealMode::OLDEST;
        static i32 currHoveredItem = 0;
        static i32 prevHoveredItem = 0;
        i32 hoverItemCounter = 0;
        enum {
            SUB_MENU_NONE = 0,
            SUB_MENU_SINGLE_PLAYER,
            SUB_MENU_MULTIPLAYER,
            SUB_MENU_MAP_EDITOR,
            SUB_MENU_OPTIONS,
            SUB_SUB_MENU_OPTIONS_GRAPHICS,
            SUB_SUB_MENU_OPTIONS_AUDIO,
        };

        static i32 subMenu = 0;
        static i32 subSubMenu = 0;

        if( nk_begin( ctx, "SubMainMenu", nk_rect( startX + 5, startY + 30 + 5, winWidth - 10, winHeight - 30 - 10 ), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ) ) {
            nk_layout_row_dynamic( ctx, ( winHeight - 30 ) / 7.58f, 1 );
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "Single Player" ) ) {
                core->AudioPlay( sndClick );
                GameStartParams startParms = {};
                startParms.mapName = "";
                startParms.isMutliplayer = false;
                startParms.localPlayerNumber = 1;
                startParms.otherPlayerNumber = -1;
                core->MoveToGameMode( new GameMode_SinglePlayerGame( startParms ) );
            }
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "MultiPlayer" ) ) {
                core->AudioPlay( sndClick );
                if ( core->NetworkIsConnected() == false ) {
                    core->NetworkConnect();
                }
            }
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "Map Editor" ) ) {
                core->AudioPlay( sndClick );
            }
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "Options" ) ) {
                core->AudioPlay( sndClick );
                subMenu = SUB_MENU_OPTIONS;
            }
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "Credits" ) ) {
                core->AudioPlay( sndClick );
            }
            ++hoverItemCounter;
            if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
            if( nk_button_label( ctx, "Quit" ) ) {
                core->AudioPlay( sndClick );
                core->WindowClose();
            }
        }
        nk_end( ctx );

        
        if ( subMenu == SUB_MENU_OPTIONS ) {
            if ( nk_begin( ctx, "SubOptions", nk_rect( startX + 5 + winWidth, startY + 30 + 5, winWidth - 10, winHeight - 30 - 10 ), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ) ) {
                nk_layout_row_dynamic( ctx, ( winHeight - 30 ) / 7.58f, 1 );
                ++hoverItemCounter;
                if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
                if( nk_button_label( ctx, "Graficss" ) ) {
                    core->AudioPlay( sndClick );
                }
                ++hoverItemCounter;
                if( nk_widget_is_hovered( ctx ) ) { currHoveredItem = hoverItemCounter; }
                if( nk_button_label( ctx, "Audio" ) ) {
                    core->AudioPlay( sndClick );
                    subSubMenu = SUB_SUB_MENU_OPTIONS_AUDIO;
                }
            }
            nk_end( ctx );

            if( subSubMenu == SUB_SUB_MENU_OPTIONS_AUDIO ) {
                f32 subWinWidth = 740; //winWidth * 3.2f - 10;
                if( nk_begin( ctx, "SubOptionsAudio", nk_rect( startX + 5 + winWidth * 2, startY + 30 + 5, subWinWidth, winHeight - 30 - 10 ), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ) ) {
                    static const float ratio[] = { subWinWidth * 0.30f, subWinWidth * 0.6f, subWinWidth * 0.1f };
                    static int masterVol = 0;
                    static int musicVol = 0;
                    static int effectsVol = 0;

                    nk_layout_row( ctx, NK_STATIC, 30, 3, ratio );
                    nk_label( ctx, "Master Volume", NK_TEXT_LEFT );
                    nk_slider_int( ctx, 0, &masterVol, 100, 1 );
                    nk_label( ctx, StringFormat::Small( "%d", masterVol ).GetCStr(), NK_TEXT_CENTERED );

                    nk_label( ctx, "Music Volume", NK_TEXT_LEFT );
                    nk_slider_int( ctx, 0, &musicVol, 100, 1 );
                    nk_label( ctx, StringFormat::Small( "%d", musicVol ).GetCStr(), NK_TEXT_CENTERED );

                    nk_label( ctx, "Effects Volume", NK_TEXT_LEFT );
                    nk_slider_int( ctx, 0, &effectsVol, 100, 1 );
                    nk_label( ctx, StringFormat::Small( "%d", effectsVol ).GetCStr(), NK_TEXT_CENTERED );
                }
                nk_end( ctx );
            }
        }

        if( nk_window_is_any_hovered( ctx ) == 0 && core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
            subMenu = SUB_MENU_NONE;
            subSubMenu = SUB_MENU_NONE;
        }

        if ( currHoveredItem != prevHoveredItem ) {
            prevHoveredItem = currHoveredItem;
            static AudioResource * sndHover = ResourceGetAndCreateAudio( "res/game/gui/main_menu_hover.wav", { true, false, 0, 0 } );
            sndHover->maxInstances = 10;
            sndHover->stealMode = AudioStealMode::OLDEST;
            core->AudioPlay( sndHover );
        }

        //overview( ctx );
        nk_glfw3_render( &nkGlfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER );
    }

}
