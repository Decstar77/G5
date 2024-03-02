#include "atto_ui.h"
#include "atto_core.h"
#include "atto_colors.h"

//#include "glad/glad.h"


namespace atto {

    void UIContext::Initialize( Core * core ) {
        this->core = core;
    }

    void UIContext::BeginBox( float startX, float startY, bool horizontal ) {
        this->startX = startX;
        this->startY = startY;
        this->horizontal = horizontal;
    }

    void UIContext::EndBox() {
    }

    bool UIContext::AddButton( const char * text ) {
        FontHandle fontHandle = core->ResourceGetFont( "default" );
        UIElement el = {};
        el.text = text;
        core->FontGetTextBounds( fontHandle, 32, text, glm::vec2( startX, startY ), el.textBounds );

        const f32 w = el.textBounds.GetWidth();
        const f32 h = el.textBounds.GetHeight();

        el.pos = glm::vec2( startX, startY );

        startY += h + 13;
        bool clicked = false;
        glm::vec2 mousePos = core->InputMousePosPixels();
        if( el.textBounds.Contains( mousePos ) ) {
            el.isHovered = true;
            if( core->InputMouseButtonDown( MouseButton::MOUSE_BUTTON_1 ) ) {
                el.isPressed = true;
            }
            if( core->InputMouseButtonJustReleased( MouseButton::MOUSE_BUTTON_1 ) ) {
                clicked = true;
            }
        }

        elements.Add( el );

        return clicked;
    }

    void UIContext::Draw( DrawContext * drawContext ) {
        FontHandle fontHandle = core->ResourceGetFont( "default" );


        const i32 elCount = elements.GetCount();
        for( int elementIndex = 0; elementIndex < elCount; elementIndex++ ) {
            const UIElement & el = elements[ elementIndex ];
            glm::vec4 backColor = Colors::MIDNIGHT_BLUE;
            if( el.isHovered ) {
                backColor *= 1.2f;
            }
            if( el.isPressed ) {
                backColor *= 1.2f;
            }

            drawContext->DrawRectScreen( el.textBounds.min, el.textBounds.max, backColor );
            drawContext->DrawText2D( fontHandle, el.pos, 32, el.text.GetCStr() );
        }

        //drawContext->DrawRectScreen( glm::vec2( 50, 0 ), glm::vec2( 100, 100 ) );

        elements.Clear();
    }

}