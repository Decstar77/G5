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

    struct Tile {
        i32 flatIndex;
        i32 xIndex;
        i32 yIndex;
        i32 spriteXIndex;
        i32 spriteYIndex;
        i32 flags;
        glm::vec2 wsCenter;
        BoxBounds2D wsBounds;
        SpriteResource * spriteResource;
    };

    constexpr i32 TILE_MAX_X_COUNT = 96;
    constexpr i32 TILE_MAX_Y_COUNT = 64;
    constexpr i32 TILE_SIZE = 8;

    class TileMap {
    public:
        void PlaceTile( i32 xIndex, i32 yIndex, SpriteResource * sprite, i32 tileXIndex, i32 tileYIndex, i32 flags );
        void FillBorder( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags );
        void Fill( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags );

        FixedList<Tile, TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT> tiles = {};
    };

    typedef ObjectHandle<VisEntity> VisEntityHandle;
    struct VisEntity {
        VisEntityHandle                 handle;
        SpriteAnimator                  spriteAnimator;
        FacingDirection                 facingDir;
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
            };
            SpriteResource*             spriteBank[16];
        };
    };

    class VisMap : public SimMap {
    public:
        void                                        VisInitialize( Core * core );
        void                                        VisUpdate( f32 dt );

        virtual VisEntity *                         VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) override;

        Core *                                          core = nullptr;
        f32                                             visTime = 0;
        glm::vec2                                       cameraPos = glm::vec2( 0, 0 );
        PlayerNumber                                    playerNumber = PlayerNumber::Create( -1 );
        TeamNumber                                      teamNumber = TeamNumber::Create(-1);
        InputMode                                       inputMode = {};
        glm::vec2                                       inputDragStart = {};
        glm::vec2                                       inputDragEnd = {};
        GrowableList<EntityHandle>                      inputDragSelection = {};
        MapActionBuffer                                 visActionBuffer = {};
        FixedObjectPool<VisEntity, MAX_ENTITY_COUNT>    visEntityPool = {};
        TileMap                                         tileMap = {};
    };
}


