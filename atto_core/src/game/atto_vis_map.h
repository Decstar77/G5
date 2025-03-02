#pragma once

#include "../sim/atto_sim_map.h"
#include "../sim/atto_sim_actions.h"

#include "atto_core.h"

namespace atto {
 
    enum class InputMode {
        NONE, 
        DRAGGING,
        PLACING_STRUCTURE,
    };

    struct GUIGamePanel {
        i32                                 activeIndex;
        SpriteResource *                    texture;
        glm::vec2                           pos;
        FixedList< glm::vec2, 9 >           imageBls;
        FixedList< glm::vec4, 9 >           imageColors;
        FixedList< SpriteResource *, 9 >    images;
    };

    struct GUIGamePanelSmol {
        SpriteResource *                    texture;
        glm::vec2                           pos;
        SpriteResource *                    leftImage;
        glm::vec2                           leftBl;
    };

    class VisMap : public SimMap {
    public:
        void                                        VisInitialize( Core * core );
        void                                        VisUpdate( f32 dt );

        virtual void                                VisMap_OnSpawnEntity( SimEntity * entity, EntitySpawnCreateInfo createInfo ) override;
        bool                                        OnGuiPanelDraw( GUIGamePanel * panel, DrawContext * guiLayer, glm::vec2 mousePosGui );
        bool                                        OnGuiPanelDrawSmol( GUIGamePanelSmol * panel, DrawContext * guiLayer, glm::vec2 mousePosGui );
        void                                        OnGUILeftPanelUpdate();
        void                                        OnGUIPanelHoverd( DrawContext * guiLayer, i32 idx );
        void                                        OnGUIPanelClicked( DrawContext * guiLayer, i32 idx );

        Core *                                          core = nullptr;
        MapActionBuffer                                 actionBuffer = {};
        glm::vec2                                       cameraPos = glm::vec2( 0, 0 );
        PlayerNumber                                    playerNumber = PlayerNumber::Create( -1 );
        TeamNumber                                      teamNumber = TeamNumber::Create(-1);
        GrowableList<EntityHandle>                      currentSelection = {};
        InputMode                                       inputMode = {};
        glm::vec2                                       inputDragStart = {};
        glm::vec2                                       inputDragEnd = {};
        GrowableList<EntityHandle>                      inputDragSelection = {};
        EntityType                                      inputPlacingBuildingType = {};
        SpriteResource *                                inputSprite = nullptr;
        SpriteResource *                                Gui_BlockSelection = nullptr;
        GUIGamePanel *                                  guiCurrentPanel = nullptr;
        GUIGamePanel                                    guiNothingPanel = {};
        GUIGamePanel                                    guiUnitPanel = {};
        GUIGamePanel                                    guiTownCenterPanel = {};
        GUIGamePanel                                    guiTownCenterUnlocksPanel = {};
    };
}


