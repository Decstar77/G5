#pragma once

#include "atto_math.h"
#include "atto_containers.h"

namespace atto {
    class Core;
    class DrawContext;

    enum class RESOURCE_HANDLE_FONT {};
    typedef ObjectHandle<RESOURCE_HANDLE_FONT> FontHandle;

    enum class UI_SizeType {
        NONE,
        PIXELS,
        TEXTCONTENT,
        PERCENTOFPARENT,
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
        CENTER,
        PIXELS_AND_CENTERED
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
        UI_FLAG_HOVERABLE = SetABit(1),
        UI_FLAG_CLICKABLE = SetABit(2),
    };

    enum UI_WidgetType {
        UI_WIDGET_TYPE_ROOT,
        UI_WIDGET_TYPE_BUTTON,
        UI_WIDGET_TYPE_POPUP,
        UI_WIDGET_TYPE_VBOX
    };

    struct UIWidget {
        i32 id;
        i32 flags;
        UI_WidgetType type;

        UIWidgetPos     pos;
        UIWidgetSize    size[ 2 ];
        glm::vec4       col;

        glm::vec2   computedPos;
        glm::vec2   computedSize;
        BoxBounds2D computedBounds;

        SmallString text;

        UIWidget * parent;
        FixedList<UIWidget *, 32> child; // @NOTE: Probably shouldn't do this...
    };

    class UIContext {
    public:
        void Begin( glm::vec2 dims );
        void End();
        UIWidget *  Widget( i32 id, const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, i32 flags, glm::vec4 col = glm::vec4( 1 ) );
        void        ColorBlockPix( i32 id, glm::vec2 center, glm::vec2 size, glm::vec4 col );
        void        LablePix( i32 id, const char * text, glm::vec2 center );
        bool        Button( i32 id, const char * text, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, glm::vec4 col = glm::vec4( 1 ) );
        bool        ButtonPix( i32 id, const char * text, glm::vec2 center, glm::vec2 size, glm::vec4 col = glm::vec4( 1 ) );
        //bool BeginPopup( i32 id, const char * text, glm::vec2 center, glm::vec2 size, glm::vec4 col = glm::vec4( 1 ) );
        //void EndPopup( i32 id );

        void BeginVBox( i32 id, UIWidgetPos pos, UIWidgetSize sizeX, UIWidgetSize sizeY, glm::vec4 c = glm::vec4( 1 ) );
        void EndVBox();

        void UpdateAndRender( Core * core, DrawContext * uiDraw );
        
        UIWidget * AllocWidget();
        UIWidget * FindWidgetWithId( i32 id );
        void ComputeSizes( Core * core );
        void ComputeSizeConstants( Core * core );
        void ComputeSizeChildSum( UIWidget * widget );
        void ComputeSizePercentOfParent( UIWidget * widget );
        void ComputeRelativePositions( UIWidget * widget );
        void ComputeBounds( UIWidget * widget, glm::vec2 pos );

        i32 clickedId = -1;
        i32 lastClickedId = -1;
        i32 popupOpen = -1;
        bool mouseOverAnyElements;
        FixedList< UIWidget, 32 > widgets;
        FixedQueue< UIWidget *, 32 > traversalQueue;
        FixedStack< i32, 32 > idStack;
        f32 fontSize = 12;
        FontHandle font;
    };
}

