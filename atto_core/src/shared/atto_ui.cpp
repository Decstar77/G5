#include "atto_ui.h"

//#include "glad/glad.h"


namespace atto {
    //glm::f32 FontComputeTextWidth(Font& font, const char* text) {
    //    f32 width = 0;

    //    for (i32 i = 0; text[i] != '\0'; i++) {
    //        width += (f32)(font.chars[text[i]].advance >> 6);
    //    }

    //    return width;
    //}

    //glm::f32 FontComputeTextHeight(Font& font, const char* text) {
    //    f32 height = 0;

    //    for (i32 i = 0; text[i] != '\0'; i++) {
    //        height = glm::max(height, (f32)(font.chars[text[i]].size.y));
    //    }

    //    return height;
    //}
    //
    //static float ease_out_quad(float start, float end, float t) {
    //    return start + (end - start) * (1 - (1 - t) * (1 - t));
    //}

    //static float ease_out_cubic(float start, float end, float t) {
    //    return start + (end - start) * (1 - (1 - t) * (1 - t) * (1 - t));
    //}

    //void UIConstraintAnimation::Trigger() {
    //    active = true;
    //}

    //f32 UIConstraintAnimation::Step(f32 dt) {
    //    time += dt;
    //    if (time >= 0.0f) {
    //        time = glm::min(time, duration);
    //        f32 t = time / duration;
    //        f32 value = ease_out_quad(valueA, valueB, t);
    //        if (time == duration) {
    //            active = false;
    //            time = 0.0f;
    //        }

    //        return value;
    //    }
    //    return valueA;
    //}

    //void UIColorAnimation::Trigger() {
    //    if (duration == 0.0) {
    //        return;
    //    }
    //    active = true;
    //}

    //bool UIColorAnimation::IsPlaying() {
    //    return active;
    //}

    //glm::vec4 UIColorAnimation::Step(f32 dt) {
    //    time += dt;

    //    if (time <= delay) {
    //        return valueA;
    //    }

    //    if (time >= duration + delay) {
    //        active = false;
    //        time = 0.0f;
    //        Swap(valueA, valueB);
    //        // Start because of the swap above
    //        return valueA;
    //    }

    //    f32 t = (time - delay) / duration;

    //    f32 r = ease_out_quad(valueA.r, valueB.r, t);
    //    f32 g = ease_out_quad(valueA.g, valueB.g, t);
    //    f32 b = ease_out_quad(valueA.b, valueB.b, t);
    //    f32 a = ease_out_quad(valueA.a, valueB.a, t);

    //    return glm::vec4(r, g, b, a);
    //}


    //UIElement::UIElement() {
    //}

    //f32 UIElement::GetTextWidth() {
    //    INVALID_CODE_PATH;
    //    return 0.0;
    //}

    //f32 UIElement::GetTextHeight() {
    //    INVALID_CODE_PATH;
    //    return 0.0;
    //}

    //f32 UIElement::GetImageWidth() {
    //    Assert(image != nullptr, "Image is null");
    //    if (image != nullptr) {
    //        return (f32)image->width;
    //    }
    //    return 0.0f;
    //}

    //f32 UIElement::GetImageHeight() {
    //    Assert(image != nullptr, "Image is null");
    //    if (image != nullptr) {
    //        return (f32)image->height;
    //    }
    //    return 0.0f;
    //}

    //void UIElement::TriggerColorAnimations() {
    //    colorAnim.Trigger();
    //    const i32 numChildren = children.GetCount();
    //    for (i32 childIndex = 0; childIndex < numChildren; childIndex++) {
    //        children[childIndex]->TriggerColorAnimations();
    //    }
    //}

    //void UIElement::TriggerConstraintAnimations() {
    //    xConstraint.anim.Trigger();
    //    yConstraint.anim.Trigger();
    //    widthConstraint.anim.Trigger();
    //    heightConstraint.anim.Trigger();

    //    const i32 numChildren = children.GetCount();
    //    for (i32 childIndex = 0; childIndex < numChildren; childIndex++) {
    //        children[childIndex]->TriggerConstraintAnimations();
    //    }
    //}

    //UILabel::UILabel() {

    //}

    //UILabel::UILabel(const char* text) {
    //    SetText(text);
    //}

    //void UILabel::SetText(const char* text) {
    //    ResourceRegistry& rr = OsGetResourceRegistry();
    //    this->text = text;
    //    this->computedTextWidth = FontComputeTextWidth(rr.arialFont_24, text);
    //    this->computedTextHeight = FontComputeTextHeight(rr.arialFont_24, text);
    //}

    //void UILabel::SetText(const SmallString& text) {
    //    SetText(text.GetCStr());
    //}
   
    //f32 UILabel::GetTextWidth() {
    //    return computedTextWidth;
    //}

    //f32 UILabel::GetTextHeight() {
    //    return computedTextHeight;
    //}

    //void UILabel::UpdateAndRender(BoxBounds bounds, f32 dt) {
    //    RenderState& rs = OsGetRenderState();
    //    ResourceRegistry& rr = OsGetResourceRegistry();
    //    Font& font = rr.arialFont_24;

    //    if (colorAnim.IsPlaying()) {
    //        color = colorAnim.Step(dt);
    //    }

    //    if (color.a != 0.0f) {
    //        glm::vec2 padding = glm::vec2(0);
    //        glm::vec2 textPos = glm::vec2(0, 0);
    //        textPos = bounds.min + glm::vec2(padding.x, bounds.GetHeight() - padding.y);

    //        DrawText(rs, font, text.GetCStr(), textPos, color);
    //    }
    //}

    //UIButton::UIButton() {
    //    color = glm::vec4(1);
    //}

    //UIButton::UIButton(const char* text) {
    //    ResourceRegistry& rr = OsGetResourceRegistry();

    //    this->text = text;
    //    this->computedTextWidth = FontComputeTextWidth(rr.arialFont_24, text);
    //    this->computedTextHeight = FontComputeTextHeight(rr.arialFont_24, text);
    //}

    //UIButton::UIButton(TextureResource* image) {
    //    this->image = image;
    //    this->color = glm::vec4(1);
    //}

    //f32 UIButton::GetTextWidth() {
    //    return computedTextWidth + textPadding.x;
    //}

    //f32 UIButton::GetTextHeight() {
    //    return computedTextHeight + textPadding.y;
    //}

    //void UIButton::UpdateAndRender(BoxBounds bounds, f32 dt) {
    //    FrameInput& fs = OsGetInput();
    //    ResourceRegistry& rr = OsGetResourceRegistry();
    //    AudioState& as = OsGetAudioState();
    //    Font& font = rr.arialFont_24;
    //    
    //    const glm::vec4 baseColor = color;
    //    const glm::vec4 hoverColor = baseColor * 2.0f;
    //    const glm::vec4 clickColor = baseColor * 3.0f;
    //    const glm::vec4 disabledColor = baseColor * 0.5f;
    //    glm::vec4 useColor = baseColor;

    //    if (bounds.Contains(fs.mousePosPixels)) {
    //        if (isHovered == false && onHoverEnter != nullptr) {
    //            AudioPlay(as, &rr.sfxD03);
    //            onHoverEnter(this);
    //        }

    //        useColor = hoverColor;
    //        isHovered = true;

    //        if (IsMouseDown(fs, MOUSE_BUTTON_1)) {
    //            useColor = clickColor;
    //        }

    //        if (IsMouseJustUp(fs, MOUSE_BUTTON_1)) {
    //            AudioPlay(as, &rr.sfxD05);
    //            if (onClick != nullptr) {
    //                onClick(this);
    //            }
    //        }
    //    }
    //    else {
    //        if (isHovered == true && onHoverExit) {
    //            onHoverExit(this);
    //        }
    //        isHovered = false;
    //    }

    //    if (color.a != 0.0f) {
    //        RenderState& rs = OsGetRenderState();

    //        DrawShapeRect(rs, bounds.min, bounds.max, useColor);
    //        if (image != nullptr) {
    //            DrawSprite(rs, image, bounds.GetCenter(), bounds.GetSize());
    //        }

    //        if (text.GetLength() > 0) {
    //            glm::vec2 padding = glm::vec2(0);
    //            glm::vec2 textPos = glm::vec2(0, 0);
    //            textPos = bounds.min + glm::vec2(
    //                bounds.GetWidth() / 2.0f - computedTextWidth / 2.0f,
    //                bounds.GetHeight() / 2.0f + computedTextHeight / 2.0f
    //            );

    //            DrawText(rs, font, text.GetCStr(), textPos, textColor);
    //        }
    //    }
    //}

    //UISlider::UISlider() {

    //}

    //UISlider::UISlider(const char* text) {
    //    ResourceRegistry& rr = OsGetResourceRegistry();

    //    this->text = text;
    //    this->computedTextWidth = FontComputeTextWidth(rr.arialFont_24, text);
    //    this->computedTextHeight = FontComputeTextHeight(rr.arialFont_24, text);
    //}

    //f32 UISlider::GetTextWidth() {
    //    return computedTextWidth;
    //}

    //f32 UISlider::GetTextHeight() {
    //    return computedTextHeight;
    //}

    //void UISlider::UpdateAndRender(BoxBounds bounds, f32 dt) {
    //    FrameInput& fs = OsGetInput();
    //    ResourceRegistry& rr = OsGetResourceRegistry();
    //    AudioState& as = OsGetAudioState();
    //    RenderState& rs = OsGetRenderState();
    //    Font& font = rr.arialFont_24;

    //    const glm::vec4 baseColor = color;
    //    const glm::vec4 hoverColor = baseColor * 2.0f;
    //    const glm::vec4 clickColor = baseColor * 3.0f;
    //    const glm::vec4 disabledColor = baseColor * 0.5f;

    //    glm::vec2 textPos = bounds.min + glm::vec2(0.0f, computedTextHeight);
    //    DrawText(rs, font, text.GetCStr(), textPos, textColor);

    //    f32 spacing = 0.35f;
    //    f32 midPoint = bounds.min.y + bounds.GetHeight() / 2.0f;
    //    glm::vec2 lineStart = glm::vec2(bounds.max.x - spacing * bounds.GetWidth(), midPoint);
    //    glm::vec2 lineEnd = glm::vec2(bounds.max.x, midPoint);
    //    DrawShapeLine(rs, lineStart, lineEnd, 5.0f, color);

    //    f32 pPixels = Lerp(lineStart.x, lineEnd.x, value);
    //    glm::vec2 draggerStart = glm::vec2(pPixels, midPoint - computedTextHeight * 0.75f);
    //    glm::vec2 draggerEnd = glm::vec2(pPixels, midPoint + computedTextHeight * 0.75f);
    //    BoxBounds dragger = DrawShapeLine(rs, draggerStart, draggerEnd, 7.0f, isDragging ? clickColor : hoverColor);
    //    
    //    dragger.Expand(1.2f);   // NOTE: this is to make the dragger easier to click on

    //    if (dragger.Contains(fs.mousePosPixels)) {
    //        if (IsMouseJustDown(fs, MOUSE_BUTTON_1)) {
    //            isDragging = true;
    //        }
    //    }

    //    if (isDragging && IsMouseDown(fs, MOUSE_BUTTON_1)) {
    //        f32 t = glm::max(0.0f, glm::min(1.0f, (fs.mousePosPixels.x - lineStart.x) / (lineEnd.x - lineStart.x)));
    //        value = t;
    //    }

    //    if (IsMouseJustUp(fs, MOUSE_BUTTON_1)) {
    //        isDragging = false;
    //    }
    //}


    //UICheckbox::UICheckbox() {

    //}

    //UICheckbox::UICheckbox(const char* text) {
    //    ResourceRegistry& rr = OsGetResourceRegistry();

    //    this->text = text;
    //    this->computedTextWidth = FontComputeTextWidth(rr.arialFont_24, text);
    //    this->computedTextHeight = FontComputeTextHeight(rr.arialFont_24, text);
    //}

    //f32 UICheckbox::GetTextWidth() {
    //    return computedTextWidth;
    //}

    //f32 UICheckbox::GetTextHeight() {
    //    return computedTextHeight;
    //}

    //void UICheckbox::UpdateAndRender(BoxBounds bounds, f32 dt) {
    //    FrameInput& fs = OsGetInput();
    //    ResourceRegistry& rr = OsGetResourceRegistry();
    //    AudioState& as = OsGetAudioState();
    //    RenderState& rs = OsGetRenderState();
    //    Font& font = rr.arialFont_24;

    //    const glm::vec4 baseColor = color;
    //    const glm::vec4 hoverColor = baseColor * 2.0f;
    //    const glm::vec4 clickColor = baseColor * 3.0f;
    //    const glm::vec4 disabledColor = baseColor * 0.5f;
    //    glm::vec4 useColor = baseColor;

    //    glm::vec2 textPos = bounds.min + glm::vec2(0.0f, computedTextHeight);
    //    DrawText(rs, font, text.GetCStr(), textPos, textColor);

    //    f32 spacing = 0.9f;
    //    f32 midPoint = bounds.min.y + bounds.GetHeight() / 2.0f;
    //    glm::vec2 centerPoint = glm::vec2(bounds.min.x + spacing * bounds.GetWidth(), midPoint);
    //    
    //    BoxBounds box = DrawShapeRect(rs,
    //        centerPoint - glm::vec2(0.5f * computedTextHeight),
    //        centerPoint + glm::vec2(0.5f * computedTextHeight),
    //        useColor
    //    );

    //    if (box.Contains(fs.mousePosPixels)) {
    //        if (IsMouseJustDown(fs, MOUSE_BUTTON_1)) {
    //            //isClicked = true;
    //            isChecked = !isChecked;
    //        }
    //    }

    //    if (isChecked) {
    //        BoxBounds box = DrawShapeRect(rs,
    //            centerPoint - glm::vec2(0.32f * computedTextHeight),
    //            centerPoint + glm::vec2(0.32f * computedTextHeight),
    //            glm::vec4(0.9f, 0.9f, 0.9f, 1.0f)
    //        );
    //    }
    //}

    //void UIBlock::UpdateAndRender(BoxBounds bounds, f32 dt) {
    //    if (color.a != 0.0f) {
    //        RenderState& rs = OsGetRenderState();
    //        DrawShapeRect(rs, bounds.min, bounds.max, color);
    //    }

    //    const i32 numChildren = children.GetCount();
    //    for (i32 i = 0; i < numChildren; i++) {
    //        UIElement *child = children[i].get();
    //        
    //        f32 width = 0.0;
    //        switch (child->widthConstraint.type) {
    //            case UISizeConstraintType::NONE: {
    //                
    //            } break;
    //            case UISizeConstraintType::PIXEL: {
    //                width = child->widthConstraint.value;
    //            } break;
    //            case UISizeConstraintType::PARENT_PERCENTAGE: {
    //                width = bounds.GetWidth() * child->widthConstraint.value;
    //            } break;
    //            case UISizeConstraintType::ASPECT: {
    //                INVALID_CODE_PATH;
    //            } break;
    //            case UISizeConstraintType::TEXT: {
    //                width = child->GetTextWidth();
    //            } break;
    //            default:
    //                INVALID_CODE_PATH;
    //        }
    //        
    //        f32 height = 0.0;
    //        switch (child->heightConstraint.type) {
    //            case UISizeConstraintType::NONE: {

    //            } break;
    //            case UISizeConstraintType::PIXEL: {
    //                height = child->heightConstraint.value;
    //            } break;
    //            case UISizeConstraintType::PARENT_PERCENTAGE: {
    //                height = bounds.GetHeight() * child->heightConstraint.value;
    //            } break;
    //            case UISizeConstraintType::ASPECT: {
    //                height = width * child->heightConstraint.value;
    //            } break;
    //            case UISizeConstraintType::TEXT: {
    //                height = child->GetTextHeight();
    //            } break;
    //            default:
    //                INVALID_CODE_PATH;
    //        }

    //        f32 xPos = 0.0f;
    //        if (child->xConstraint.anim.active) {
    //            child->xConstraint.value = child->xConstraint.anim.Step(dt);
    //        }

    //        switch (child->xConstraint.type) {
    //            case UIPosConstraintType::NONE: {
    //                
    //            } break;
    //            case UIPosConstraintType::PIXEL: {
    //                xPos = bounds.min.x + child->xConstraint.value;
    //            } break;
    //            case UIPosConstraintType::CENTER: {
    //                xPos = bounds.min.x + bounds.GetWidth() / 2.0f - width / 2.0f;
    //            } break;
    //            case UIPosConstraintType::PARENT_PERCENTAGE: {
    //                xPos = bounds.min.x + bounds.GetWidth() * child->xConstraint.value;
    //            } break;
    //            default:
    //                INVALID_CODE_PATH
    //        }

    //        f32 yPos = 0.0f;
    //        switch (child->yConstraint.type)
    //        {
    //            case UIPosConstraintType::NONE: {

    //            } break;
    //            case UIPosConstraintType::PIXEL: {
    //                yPos = bounds.min.y + child->yConstraint.value;
    //            } break;
    //            case UIPosConstraintType::CENTER: {
    //                yPos = bounds.min.y + bounds.GetHeight() / 2.0f - height / 2.0f;
    //            } break;
    //            case UIPosConstraintType::PARENT_PERCENTAGE: {
    //                yPos = bounds.min.y + bounds.GetHeight() * child->yConstraint.value;
    //            } break;
    //            default:
    //                INVALID_CODE_PATH
    //        }

    //        BoxBounds childBounds = {};
    //        childBounds.min = glm::vec2(xPos, yPos);
    //        childBounds.max = glm::vec2(xPos + width, yPos + height);

    //        child->UpdateAndRender(childBounds, dt);
    //    }
    //}

    //UIMaster::UIMaster() {
    //    
    //}

    //void UIMaster::UpdateAndRender(RenderState& rs, f32 dt) {
    //    glm::vec2 size = glm::vec2(rs.mainSurfaceWidth, rs.mainSurfaceHeight);

    //    BoxBounds bounds = {};
    //    bounds.min = glm::vec2(0.0f, 0.0f);
    //    bounds.max = size;

    //    block.UpdateAndRender(bounds, dt);
    //}



}