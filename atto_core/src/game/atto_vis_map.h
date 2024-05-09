#pragma once

#include "../sim/atto_sim_map.h"
#include "../sim/atto_sim_actions.h"

#include "atto_core.h"

namespace atto {
    enum class FacingDirection {
        RIGHT = 0,
        LEFT,
        UP,
        DOWN
    };

    enum class InputMode {
        NONE, 
        DRAGGING,
        PLACING_STRUCTURE,
    };

    typedef ObjectHandle<VisEntity> VisEntityHandle;
    struct VisEntity {
        VisEntityHandle                 handle;
        SpriteAnimator                  spriteAnimator;
        FacingDirection                 facingDir;
        glm::vec2                       spriteSelectionOffset;
        AudioGroupResource *            sndOnSelect;
        AudioGroupResource *            sndOnMove;
        AudioGroupResource *            sndOnAttack;
        union {
            struct {
                SpriteResource *        spriteBankIdleSide;
                SpriteResource *        spriteBankIdleDown;
                SpriteResource *        spriteBankIdleUp;
                SpriteResource *        spriteBankWalkSide;
                SpriteResource *        spriteBankWalkDown;
                SpriteResource *        spriteBankWalkUp;
                SpriteResource *        spriteBankAttackSide;
                SpriteResource *        spriteBankAttackDown;
                SpriteResource *        spriteBankAttackUp;
                SpriteResource *        spriteBankDeath;
                SpriteResource *        spriteBankSelection;
            };
            SpriteResource*             spriteBank[16];
        };
    };

    class VisMap : public SimMap {
    public:
        void                                        VisInitialize( Core * core );
        void                                        VisUpdate( f32 dt );

        virtual VisEntity *                         VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) override;

        void                                        OnGUILeftPanelUpdate();
        void                                        OnGUILeftPanelClicked( i32 idx );

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
        FixedObjectPool<VisEntity, MAX_ENTITY_COUNT>    visEntityPool = {};
    };
}


