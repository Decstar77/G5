#pragma once

#include "atto_sim_var.h"
#include "atto_sim_actions.h"

namespace atto {
    struct SimEntity;
    struct VisEntity;

    enum class EntityType {
        INVALID = 0,
        UNIT_SCOUT,

        STRUCTURE_CITY_CENTER,
    };

    enum class UnitState {
        IDLE = 0,
        MOVING = 1,
        ATTACKING = 2
    };

    struct Unit {
        UnitState state;
    };

    inline bool IsUnitType( EntityType t ) { return t == EntityType::UNIT_SCOUT; }

    struct SimStreamData {
        EntityHandle handle;
        glm::vec2 pos;
        bool isAttacking;
    };

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;
        PlayerNumber                playerNumber;
        TeamNumber                  teamNumber;
        FixedList<PlayerNumber, 4>  selectedBy;
        glm::vec2                   lastPos;
        glm::vec2                   pos;
        PosTimeline                 posTimeline;
        Collider2D                  collider;
        Unit                        unit;
        glm::vec2                   dest;
        EntityHandle                target;
        VisEntity *                 vis;

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
        i32                                          streamDataCounter = 0;
        FixedList<SimStreamData, MAX_ENTITY_COUNT>   streamData = {};

    protected:
        virtual VisEntity *     VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) { return nullptr; }

    private:
        void        Action_RequestMove( PlayerNumber playerNumber, glm::vec2 p );
        void        Action_RequestAttack( PlayerNumber playerNumber, EntityHandle handle );
        void        Action_RequestSelectEntities( PlayerNumber playerNumber, Span<EntityHandle> entities );

        SimEntity * Action_CommandSpawnEntity( EntitySpawnCreateInfo createInfo );
        SimEntity * Action_CommandSpawnEntity( EntityType type, glm::vec2 pos, PlayerNumber playerNumber, TeamNumber teamNumber );
        void        Action_CommandMove( EntityHandle entityHandle, glm::vec2 p1, glm::vec2 p2, f32 t1, f32 t2 );

        f32                                         simTime = 0;
        f32                                         simTimeAccum = 0;
        FixedList<SimEntity *, MAX_ENTITY_COUNT>    actionActiveEntities = {};
    };
}

