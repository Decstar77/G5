#include "atto_ui.h"
#include "atto_core.h"
#include "atto_colors.h"

//#include "glad/glad.h"
#include <iostream>
namespace atto {

    void UIContext::Begin( glm::vec2 dims ) {
        idStack.Push( 0 );

        UIWidget * w = AllocWidget();
        w->id = 0;
        w->pos.type = UI_PosType::PIXELS;
        w->size[UI_AXIS_X].type = UI_SizeType::PIXELS;
        w->size[UI_AXIS_X].value = dims.x;
        w->size[UI_AXIS_Y].type = UI_SizeType::PIXELS;
        w->size[UI_AXIS_Y].value = dims.y;
    }

    void UIContext::End() {
        widgets.Clear();
        idStack.Clear();
    }
    
    UIWidget * UIContext::Widget( i32 id, const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, i32 flags, glm::vec4 col ) {
        UIWidget * w = AllocWidget();
        w->id = id;
        w->flags = flags;
        w->pos = pos;
        w->size[UI_AXIS_X] = sizeX;
        w->size[UI_AXIS_Y] = sizeY;
        w->col = col;
        w->text = SmallString::FromLiteral( text );
        w->parent = FindWidgetWithId( *idStack.Peek() );
        Assert( w->parent != nullptr );
        if ( w->parent != nullptr ) {
            w->parent->child.Add( w );
        }

        return w;
    }

    void UIContext::ColorBlockPix( i32 id, glm::vec2 center, glm::vec2 size, glm::vec4 col ) {
        UIWidgetPos pos = {};
        pos.type = UI_PosType::PIXELS;
        pos.value = center;
        pos.value.x -= size.x / 2.0f;
        pos.value.y += size.y / 2.0f;
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::PIXELS;
        sizeX.value = size.x;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::PIXELS;
        sizeY.value = size.y;
        Widget( id, "", pos, sizeX, sizeY, 0, col );
    }

    void UIContext::Seperator( i32 id ) {
        UIWidgetPos pos = {};
        pos.type = UI_PosType::CENTER;
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::PERCENT_OF_PARENT;
        sizeX.value = 1.0f;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::PIXELS;
        sizeY.value = 1.0f;
        Widget( id, "", pos, sizeX, sizeY, 0, glm::vec4( 1, 1, 1, 1 ) );
    }

    void UIContext::Lable( const char * text, bool center ) {
        UIWidgetPos pos = {};
        pos.type = center ? UI_PosType::CENTER : UI_PosType::ALIGNED_LEFT;
        UIWidgetSize sizeX = {};
        sizeX.type = center ? UI_SizeType::PERCENT_OF_PARENT : UI_SizeType::TEXTCONTENT;
        sizeX.value = 1;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::TEXTCONTENT;
        i32 id = ( i32 )StringHash::Hash( text );
        Widget( id, text, pos, sizeX, sizeY, 0, glm::vec4( 0 ) );
    }

    void UIContext::Lable( const SmallString & s, bool center ) {
        Lable( s.GetCStr(), center );
    }

    void UIContext::LablePix( const char * text, glm::vec2 center ) {
        UIWidgetPos pos = {};
        pos.type = UI_PosType::PIXELS;
        pos.value = center;
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::TEXTCONTENT;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::TEXTCONTENT;
        i32 id = ( i32 )StringHash::Hash( text );
        Widget( id, text, pos, sizeX, sizeY, UI_FLAG_TREAT_POS_AS_CENTER, glm::vec4( 0 ) );
    }

    void UIContext::Image( i32 id, TextureResource * image ) {
        UIWidgetPos pos = {};
        pos.type = UI_PosType::CENTER;
        glm::vec2 size = glm::vec2( image->width, image->height );
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::PIXELS;
        sizeX.value = size.x;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::PIXELS;
        sizeY.value = size.y;
        UIWidget * w = Widget( id, "", pos, sizeX, sizeY, UI_FLAG_TREAT_POS_AS_CENTER, glm::vec4( 0 ) );
        w->image = image;
    }

    bool UIContext::Button( const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, glm::vec4 col ) {
        i32 id = ( i32 )StringHash::Hash( text );
        UIWidget * w = Widget( id, text, pos, sizeX, sizeY, UI_FLAG_HOVERABLE | UI_FLAG_CLICKABLE, col );
        return w->id == clickedId;
    }

    bool UIContext::ButtonPix( i32 id, const char * text, glm::vec2 center, glm::vec2 size, glm::vec4 col ) {
        UIWidgetPos pos = {};
        pos.type = UI_PosType::PIXELS;
        pos.value = center;
        pos.value.x -= size.x / 2.0f;
        pos.value.y += size.y / 2.0f;
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::PIXELS;
        sizeX.value = size.x;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::PIXELS;
        sizeY.value = size.y;
        UIWidget * w = Widget( id, text, pos, sizeX, sizeY, UI_FLAG_HOVERABLE | UI_FLAG_CLICKABLE, col );
        return w->id == clickedId;
    }

    bool UIContext::Slider( const char * text, f32 * value, glm::vec4 col ) {
        i32 id =( i32 )StringHash::Hash( text );
        UIWidgetPos pos = {};
        pos.type = UI_PosType::CENTER;
        UIWidgetSize sizeX = {};
        sizeX.type = UI_SizeType::PIXELS;
        sizeX.value = 100;
        UIWidgetSize sizeY = {};
        sizeY.type = UI_SizeType::PIXELS;
        sizeY.value = 10;
        UIWidget * w = Widget( id, text, pos, sizeX, sizeY, UI_FLAG_HOVERABLE | UI_FLAG_CLICKABLE, col );
        w->slider.minValue = 0.0f;
        w->slider.maxValue = 1.0f;
        w->type = UI_WIDGET_TYPE_SLIDER;

        if ( id == pressedId ) {
            *value = normalizedHoverPos.x;
        }
        w->slider.t = *value;

        return false;
    }

    void UIContext::BeginVBox( i32 id, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, i32 flags, glm::vec4 c ) {
        UIWidget * w = Widget( id, "", pos, sizeX, sizeY, flags, c );
        w->type = UI_WIDGET_TYPE_VBOX;

        idStack.Push( id );
    }

    void UIContext::EndVBox() {
        idStack.Pop();
    }

    void UIContext::UpdateAndRender( Core * core, DrawContext * uiDraw ) {
        fontHandle = core->ResourceGetFont( "ken" ); // @HACK

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePos = uiDraw->ScreenPosToWorldPos( mousePosPix );
        const bool mousePressed = core->InputMouseButtonDown( MOUSE_BUTTON_1 );
        const bool mouseClicked = core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 );


        if ( widgets.IsEmpty() == true ) {
            return;
        }

        ComputeSizes( core );

        if (mouseClicked == true ) {
            //lastClickedId = -1;
        }

        mouseOverAnyElements = false;
        clickedId = -1;
        pressedId = -1;
        hoverId = -1;
        traversalQueue.Clear();
        traversalQueue.Enqueue( &widgets[ 0 ] );
        while ( traversalQueue.IsEmpty() == false ) {
            UIWidget * widget = traversalQueue.Dequeue();
            for ( i32 i = 0; i < widget->child.GetCount(); i++ ) {
                UIWidget * child = widget->child[ i ];

                glm::vec4 col = child->col;
                if ( EnumHasFlag( child->flags, UI_FLAG_HOVERABLE ) && child->computedBounds.Contains( mousePos ) == true ) {
                    mouseOverAnyElements = true;
                    hoverId = child->id;
                    normalizedHoverPos = ( mousePos - child->computedBounds.min ) / child->computedBounds.GetSize() ;
                    col *= 1.1f;

                    if ( EnumHasFlag( child->flags, UI_FLAG_CLICKABLE ) && mousePressed == true ) {
                        col *= 1.1f;
                        pressedId = child->id;
                    }

                    if ( EnumHasFlag( child->flags, UI_FLAG_CLICKABLE ) && mouseClicked == true ) {
                        clickedId = child->id;
                    }
                }
                
                if ( col.a != 0.0f ) {
                    uiDraw->DrawRect( child->computedBounds.min, child->computedBounds.max, col );
                }

                if ( child->image != nullptr ) { 
                    uiDraw->DrawTexture( child->image, child->computedBounds.GetCenter(), 0.0f );
                }

                if ( child->type == UI_WIDGET_TYPE_SLIDER ) {
                    glm::vec2 start = child->computedBounds.min;
                    glm::vec2 end = child->computedBounds.max;
                    f32 d = ( end.y - start.y );
                    start.y += d / 2.0f;
                    end.y -= d / 2.0f;
                    uiDraw->DrawLine( start, end, 2, glm::vec4(0, 1, 0, 1) );

                    glm::vec2 cpos = glm::mix( start, end, child->slider.t );
                    uiDraw->DrawCircle( cpos, 3.f );
                }

                if ( child->text.GetLength() != 0 ) {
                    if  ( child->pos.type == UI_PosType::ALIGNED_LEFT ) {
                        glm::vec2 padding = glm::vec2( 2, 0 ); // @HACK: PADDING
                        glm::vec2 screenPos = uiDraw->WorldPosToScreenPos( child->computedBounds.min + padding );
                        glm::vec2 screenFontSize = uiDraw->WorldPosToScreenPos( glm::vec2( 0, fontSize ) );
                        uiDraw->DrawTextScreen( fontHandle, screenPos, screenFontSize.y, child->text.GetCStr(), TextAlignment_H::FONS_ALIGN_LEFT, TextAlignment_V::FONS_ALIGN_MIDDLE );
                    } else {
                        glm::vec2 center = child->computedBounds.GetCenter();
                        glm::vec2 screenPos = uiDraw->WorldPosToScreenPos( center );
                        glm::vec2 screenFontSize = uiDraw->WorldPosToScreenPos( glm::vec2( 0, fontSize ) );
                        uiDraw->DrawTextScreen( fontHandle, screenPos, screenFontSize.y, child->text.GetCStr(), TextAlignment_H::FONS_ALIGN_CENTER, TextAlignment_V::FONS_ALIGN_MIDDLE );
                    }
                }

                traversalQueue.Enqueue( child );
            }
        }
    }

    UIWidget * UIContext::AllocWidget( ) {
        return &widgets.AddEmpty();
    }

    UIWidget * UIContext::FindWidgetWithId( i32 id ) {
        const i32 widgetCount = widgets.GetCount();
        for ( i32 widgetIndex = 0; widgetIndex < widgetCount; widgetIndex++ ) {
            UIWidget * widget = &widgets[ widgetIndex ];
            if ( widget->id == id ) {
                return widget;
            }
        }

        return nullptr;
    }

    void UIContext::ComputeSizes( Core * core ) {
        ComputeSizeConstants( core );
        ComputeSizeChildSum( widgets.Get( 0 ) );
        ComputeSizePercentOfParent( widgets.Get( 0 ) );
        ComputeRelativePositions( &widgets[ 0 ] );
        ComputeBounds( &widgets[0], glm::vec2( 0, 0 ) );
    }

    void UIContext::ComputeSizeConstants( Core * core ) {
        const i32 widgetCount = widgets.GetCount();
        for ( i32 widgetIndex = 0; widgetIndex < widgetCount; widgetIndex++ ) {
            UIWidget * widget = widgets.Get( widgetIndex );
            for ( i32 axis = 0; axis < UI_AXIS_COUNT; axis++ ) {
                UIWidgetSize size = widget->size[ axis ];
                if ( size.type == UI_SizeType::PIXELS ) {
                    widget->computedSize[ axis ] = size.value;
                }
                else if ( size.type == UI_SizeType::TEXTCONTENT ) {
                    BoxBounds2D bounds = {};
                    core->FontGetTextBounds( fontHandle, fontSize, widget->text.GetCStr(), glm::vec2( 0, 0 ), bounds, TextAlignment_H::FONS_ALIGN_CENTER, TextAlignment_V::FONS_ALIGN_MIDDLE );
                    widget->computedSize[ axis ] = bounds.GetSize()[ axis ];
                }
            }
        }
    }

    void UIContext::ComputeSizeChildSum( UIWidget * widget ) {
        for ( i32 i = 0; i < widget->child.GetCount(); i++ ) {
            UIWidget * child = widget->child[ i ];
            ComputeSizeChildSum( child );
        }

        for ( i32 axis = 0; axis < UI_AXIS_COUNT; axis++ ) {
            UIWidgetSize size = widget->size[ axis ];
            if ( size.type == UI_SizeType::CHILDRENSUM ) {
                f32 size = 0.0f;
                for ( i32 childIndex = 0; childIndex < widget->child.GetCount(); childIndex++ ) {
                    UIWidget * child = widget->child[ childIndex ];
                    size += child->computedSize[ axis ];
                }
                widget->computedSize[ axis ] = size;
            } 
            else if ( size.type == UI_SizeType::CHILDRENMAX ) {
                f32 size = 0.0f;
                for ( i32 childIndex = 0; childIndex < widget->child.GetCount(); childIndex++ ) {
                    UIWidget * child = widget->child[ childIndex ];
                    size = glm::max( size, child->computedSize[ axis ] );
                }
                widget->computedSize[ axis ] = size;
            }
        }
    }

    void UIContext::ComputeSizePercentOfParent( UIWidget * widget ) {
        for ( i32 axis = 0; axis < UI_AXIS_COUNT; axis++ ) {
            UIWidgetSize size = widget->size[ axis ];
            if ( size.type == UI_SizeType::PERCENT_OF_PARENT ) {
                widget->computedSize[ axis ] = size.value * widget->parent->computedSize[ axis ];
            }
        }

        for ( i32 i = 0; i < widget->child.GetCount(); i++ ) {
            UIWidget * child = widget->child[ i ];
            ComputeSizePercentOfParent( child );
        }
    }

    void UIContext::ComputeRelativePositions( UIWidget * widget ) {
        glm::vec2 p = glm::vec2( 0 );
        for ( i32 i = 0; i < widget->child.GetCount(); i++ ) {
            UIWidget * child = widget->child[ i ];

            if ( child->pos.type == UI_PosType::NONE ) {
            }
            else if ( child->pos.type == UI_PosType::PIXELS ) {
                if ( EnumHasFlag( child->flags, UI_FLAG_TREAT_POS_AS_CENTER ) == true ) {
                    child->computedPos = child->pos.value;
                    child->computedPos.x -= child->computedSize.x / 2.0f;
                    child->computedPos.y += child->computedSize.y / 2.0f;
                } else {
                    child->computedPos = child->pos.value;
                }
            } 
            else if ( child->pos.type == UI_PosType::ALIGNED_LEFT ) {
                child->computedPos = p;
                p.y -= child->computedSize.y;
            }
            else if ( child->pos.type == UI_PosType::CENTER ) {
                if( widget->id == 0 ) { // @HACK   
                    child->computedPos = widget->computedSize / 2.0f;
                    child->computedPos.x -= child->computedSize.x / 2.0f;
                    child->computedPos.y += child->computedSize.y / 2.0f;
                }
                else {
                    child->computedPos = p;
                    child->computedPos.x = ( widget->computedSize.x - child->computedSize.x ) / 2.0f;
                    p.y -= child->computedSize.y;
                }
            }
            else if ( child->pos.type == UI_PosType::PERCENT_OF_PARENT ) {
                glm::vec2 start = widget->computedPos;
                glm::vec2 end = widget->computedPos + widget->computedSize;
                child->computedPos.x = glm::mix( start.x, end.x, child->pos.value.x );
                child->computedPos.y = glm::mix( start.y, end.y, child->pos.value.y );

                if ( EnumHasFlag( child->flags, UI_FLAG_TREAT_POS_AS_CENTER ) == true ) {
                    child->computedPos.x -= child->computedSize.x / 2.0f;
                    child->computedPos.y += child->computedSize.y / 2.0f;
                }
            }

            ComputeRelativePositions( child );
        }
    }
    
    void UIContext::ComputeBounds( UIWidget * widget, glm::vec2 pos ) {
        glm::vec2 wp = widget->computedPos + pos;
        widget->computedBounds.min.x = wp.x;
        widget->computedBounds.min.y = wp.y - widget->computedSize.y;
        widget->computedBounds.max.x = wp.x + widget->computedSize.x;
        widget->computedBounds.max.y = wp.y;

        for ( i32 i = 0; i < widget->child.GetCount(); i++ ) {
            UIWidget * child = widget->child[ i ];
            ComputeBounds( child, wp );
        }
    }
}