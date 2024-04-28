#pragma once

#include "atto_sim_var.h"
#include "atto_sim_actions.h"

namespace atto {
    struct SimEntity;
    struct VisEntity;

    constexpr i32 MAX_ENTITY_COUNT = 2048;

    enum class EntityType {
        INVALID = 0,
        UNIT_SCOUT,

        STRUCTURE_CITY_CENTER,
    };

    inline bool IsUnitType( EntityType t ) { return t == EntityType::UNIT_SCOUT; }

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;
        PlayerNumber                playerNumber;
        TeamNumber                  teamNumber;
        VariableTimeline            posTimeline;
        VisEntity *                 vis;
        Collider2D                  collider;
        FixedList<PlayerNumber, 4>  selectedBy;

        inline Collider2D   ColliderWorldSpace( glm::vec2 p ) const { Collider2D c = collider; c.Translate( p ); return c; }
    };

    struct EntitySpawnCreateInfo {
        glm::vec2 pos;
        EntityType type;
        PlayerNumber playerNumber;
        TeamNumber teamNumber;
    };

    class SimMap {
    public:
        void        SimInitialize();
        void        SimUpdate( f32 dt );
        void        ApplyActions( MapActionBuffer * actionBuffer );

    public:
        FixedObjectPool<SimEntity, MAX_ENTITY_COUNT> entityPool = {};
        MapActionBuffer                              simActionBuffer;

    protected:
        virtual VisEntity *     VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) { return nullptr; }

    private:

        void        Action_RequestMove( PlayerNumber playerNumber, glm::vec2 p );
        void        Action_RequestSelectEntities( PlayerNumber playerNumber, Span<EntityHandle> entities );

        SimEntity * Action_CommandSpawnEntity( EntitySpawnCreateInfo createInfo );
        SimEntity * Action_CommandSpawnEntity( EntityType type, glm::vec2 pos, PlayerNumber playerNumber, TeamNumber teamNumber );
        void        Action_CommandMove( EntityHandle entityHandle, glm::vec2 p1, glm::vec2 p2, f32 t1, f32 t2 );

        f32                                         simTime = 0;
        FixedList<SimEntity *, MAX_ENTITY_COUNT>    actionActiveEntities = {};
    };
}

