#pragma once

#include "atto_math.h"
#include "atto_containers.h"

namespace atto {
    class Core;
    class DrawContext;

    class UIContext {
    public:
        void Initialize( Core * core );
        void BeginBox( float startX, float starY, bool horizontal );
        void EndBox();
        bool AddButton( const char * text );

        void Draw( DrawContext * drawContext );

    private:
        struct UIElement {
            SmallString text;
            glm::vec2   pos;
            BoxBounds2D textBounds;
            bool isHovered;
            bool isPressed;
        };

        Core *                      core;
        float   startX;
        float   startY;
        bool    horizontal;

        FixedList<UIElement, 1024>  elements;
    };
}

