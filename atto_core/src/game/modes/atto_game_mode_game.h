#pragma once

#include "../atto_game.h"

namespace atto {
    enum EntityType {
        ENTITY_TYPE_INVALID = 0,
       
    };

    constexpr static int    MAX_ENTITIES = 1024;

    struct Entity;
    typedef ObjectHandle<Entity> EntityHandle;
    typedef FixedList<Entity *, MAX_ENTITIES> EntList;

    struct Entity {
        EntityHandle    id;
        EntityType      type;
        glm::vec3       pos;
        glm::mat3       ori;

        f32                 fireRate;
        f32                 fireRateAccumulator;
        i32                 fireDamage;

        i32                 maxHealth;
        i32                 currentHealth;

        // Make these flags
        bool hasCollision;
        bool isCollisionStatic;
        bool isSelectable;

        Collider selectionCollider;
        Collider collisionCollider;

        bool selected;

        i32 teamNumber;

        Collider GetSelectionColliderWorld() const;
        Collider GetCollisionCircleWorld() const;
    };

    class GameModeGame : public GameMode {
    public:
        GameModeType GetGameModeType() override;
        void Init( Core * core ) override;
        void UpdateAndRender( Core * core, f32 dt ) override;
        void Shutdown( Core * core ) override;

    public:
        Camera                                  playerCamera = {};
        f32                                     dtAccumulator = 0.0f;
    };
}
