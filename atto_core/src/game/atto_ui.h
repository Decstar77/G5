#pragma once

#include "../shared/atto_math.h"
#include "../shared/atto_containers.h"

namespace atto {
    class Core;
    class DrawContext;
    class TextureResource;

    enum class UI_SizeType {
        NONE,
        PIXELS,
        TEXTCONTENT,
        PERCENT_OF_PARENT,
        CHILDRENSUM,
        CHILDRENMAX,
    };

    struct UIWidgetSize {
        UI_SizeType type;
        f32 value;
    };
    
    enum class UI_PosType {
        NONE,
        PIXELS,
        ALIGNED_LEFT,
        CENTER,
        PERCENT_OF_PARENT,
    };

    struct UIWidgetPos {
        UI_PosType type;
        glm::vec2 value;
    };

    enum UIAxis {
        UI_AXIS_X = 0,
        UI_AXIS_Y,
        UI_AXIS_COUNT
    };

    enum UI_Flags {
        UI_FLAG_NONE = 0,
        UI_FLAG_HOVERABLE = SetABit( 1 ),
        UI_FLAG_CLICKABLE = SetABit( 2 ),
        UI_FLAG_TREAT_POS_AS_CENTER = SetABit( 3 ),
    };

    enum UI_WidgetType {
        UI_WIDGET_TYPE_ROOT,
        UI_WIDGET_TYPE_BUTTON,
        UI_WIDGET_TYPE_POPUP,
        UI_WIDGET_TYPE_VBOX,
        UI_WIDGET_TYPE_SLIDER,
    };

    struct UISlider {
        f32 t;
        f32 maxValue;
        f32 minValue;
    };

    struct UIWidget {
        i32 id;
        i32 flags;
        UI_WidgetType type;

        UIWidgetPos     pos;
        UIWidgetSize    size[ 2 ];
        glm::vec4       col;

        TextureResource * image;

        glm::vec2   computedPos;
        glm::vec2   computedSize;
        BoxBounds2D computedBounds;

        SmallString text;
        UISlider slider;

        UIWidget * parent;
        FixedList<UIWidget *, 32> child; // @NOTE: Probably shouldn't do this...
    };

    class UIContext {
    public:
        void Begin( glm::vec2 dims );
        void End();
        UIWidget *  Widget( i32 id, const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, i32 flags, glm::vec4 col = glm::vec4( 0 ) );

        void        ColorBlockPix( i32 id, glm::vec2 center, glm::vec2 size, glm::vec4 col );

        void        Seperator( i32 id );

        void        Lable( const char * text, bool center = false );
        void        Lable( const SmallString & text, bool center = false );
        void        LablePix( const char * text, glm::vec2 center );

        void        Image( i32 id, TextureResource * image );

        bool        Button( const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, glm::vec4 col = glm::vec4( 0 ) );
        bool        ButtonPix( i32 id, const char * text, glm::vec2 center, glm::vec2 size, glm::vec4 col = glm::vec4( 0 ) );
        
        bool        Slider( const char * text, f32 * value, glm::vec4 col = glm::vec4( 0 ) );

        void        BeginVBox( i32 id, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, i32 flags = 0, glm::vec4 col = glm::vec4( 0 ) );
        void        EndVBox();

        void        UpdateAndRender( Core * core, DrawContext * uiDraw );
        
        UIWidget * AllocWidget();
        UIWidget * FindWidgetWithId( i32 id );
        void ComputeSizes( Core * core );
        void ComputeSizeConstants( Core * core );
        void ComputeSizeChildSum( UIWidget * widget );
        void ComputeSizePercentOfParent( UIWidget * widget );
        void ComputeRelativePositions( UIWidget * widget );
        void ComputeBounds( UIWidget * widget, glm::vec2 pos );

        i32 hoverId = -1;
        i32 clickedId = -1;
        i32 pressedId = -1;
        glm::vec2 normalizedHoverPos = glm::vec2( 0, 0 );
        i32 lastClickedId = -1;
        i32 popupOpen = -1;
        bool mouseOverAnyElements;
        FixedList< UIWidget, 32 > widgets;
        FixedQueue< UIWidget *, 32 > traversalQueue;
        FixedStack< i32, 32 > idStack;
        f32 fontSize = 10;
        FontHandle fontHandle;
    };
}

