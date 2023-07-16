#pragma once

#include "atto_math.h"
#include "atto_containers.h"

#include <memory>
#include <functional>

namespace atto {
    struct RenderState;
    struct TextureResource;

    class UIConstraintAnimation {
    public:
        void Trigger();
        f32 Step(f32 dt);
        f32 valueA = 0.0f;
        f32 valueB = 0.0f;
        f32 duration = 0.0f;
        f32 time = 0.0f;
        bool active;
    };

    class UIColorAnimation {
    public:
        void Trigger();
        bool IsPlaying();
        glm::vec4 Step(f32 dt);

        glm::vec4 valueA = glm::vec4(1);
        glm::vec4 valueB = glm::vec4(1);;
        f32 duration = 0.0f;
        f32 delay = 0.0f;

    private:
        bool active = false;
        f32 time = 0.0f;
    };

    enum class UIPosConstraintType {
        NONE = 0,
        PIXEL,
        CENTER,
        PARENT_PERCENTAGE,
    };

    class UIPosConstraint {
    public:
        UIPosConstraintType type = UIPosConstraintType::NONE;
        UIConstraintAnimation anim = {};
        f32 value = 0.0f;
    };

    enum class UISizeConstraintType {
        NONE = 0,
        PIXEL,
        PARENT_PERCENTAGE,
        ASPECT,
        TEXT,
        IMAGE,
    };

    class UISizeConstraint {
    public:
        UISizeConstraintType type = UISizeConstraintType::NONE;
        UIConstraintAnimation anim = {};
        f32 value = 0.0f;
    };

    class UIElement {
    public:
        UIElement();

        virtual f32 GetTextWidth();
        virtual f32 GetTextHeight();
        virtual f32 GetImageWidth();
        virtual f32 GetImageHeight();
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) = 0;

        void TriggerColorAnimations();
        void TriggerConstraintAnimations();

        UIPosConstraint  xConstraint = {};
        UIPosConstraint  yConstraint = {};
        UISizeConstraint widthConstraint = {};
        UISizeConstraint heightConstraint = {};
        UIColorAnimation colorAnim = {};
        glm::vec4 color = glm::vec4(0.0f);
        TextureResource* image = nullptr;
        FixedList<std::shared_ptr<UIElement>, 8>    children = {};

        std::function<void(UIElement *self)> onHoverEnter;
        std::function<void(UIElement* self)> onHoverExit;
        std::function<void(UIElement *self)> onClick;
        
    protected:
        bool isHovered = false;
    };

    class UILabel : public UIElement {
    public:
        UILabel();
        UILabel(const char* text);

        void SetText(const char* text);
        void SetText(const SmallString &text);

        virtual f32 GetTextWidth() override;
        virtual f32 GetTextHeight() override;
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) override;

    private:
        SmallString text = {};
        f32 computedTextWidth = 0.0f;
        f32 computedTextHeight = 0.0f;
    };

    class UIButton : public UIElement {
    public:
        UIButton();
        UIButton(const char* text);
        UIButton(TextureResource* image);

        virtual f32 GetTextWidth() override;
        virtual f32 GetTextHeight() override;
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) override;

        glm::vec4 textColor = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
        glm::vec2 textPadding = glm::vec2(13.0f);

    private:
        SmallString text = {};
        f32 computedTextWidth = 0.0f;
        f32 computedTextHeight = 0.0f;
    };

    class UISlider : public UIElement {
    public:
        UISlider();
        UISlider(const char* text);

        virtual f32 GetTextWidth() override;
        virtual f32 GetTextHeight() override;
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) override;

        f32 value = 0.0f;
        glm::vec4 textColor = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

    private:
        SmallString text = {};
        f32 computedTextWidth = 0.0f;
        f32 computedTextHeight = 0.0f;
        bool isDragging = false;
    };

    class UICheckbox : public UIElement {
    public:
        UICheckbox();
        UICheckbox(const char* text);

        virtual f32 GetTextWidth() override;
        virtual f32 GetTextHeight() override;
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) override;

        bool isChecked = false;
        glm::vec4 textColor = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
        
    private:
        SmallString text = {};
        f32 computedTextWidth = 0.0f;
        f32 computedTextHeight = 0.0f;
    };

    class UIBlock : public UIElement {
    public:
        virtual void UpdateAndRender(BoxBounds bounds, f32 dt) override;
    };

    class UIMaster {
    public:
        UIMaster();
        void UpdateAndRender(RenderState& rs, f32 dt);

        UIBlock block;
    };
}

