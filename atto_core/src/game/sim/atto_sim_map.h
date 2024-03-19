#pragma once

#include "../../shared/atto_math.h"

namespace atto {
    
    constexpr static i32 MAX_ENTITIES = 1024;
    constexpr static i32 MAX_PLAYERS = 4;

    constexpr i32 TURNS_PER_SECOND = 10;
    constexpr f32 TURNS_DELTA = 1.0f / TURNS_PER_SECOND;

    constexpr f32 MAX_MAP_SIZE = 3000.0f;

    class Core;
    class SimMap;
    struct SimEntity;
    class MapCommunicator;

    typedef ObjectHandle<SimEntity> EntityHandle;
    typedef FixedList<SimEntity *, MAX_ENTITIES> EntList;
    typedef FixedList<EntityHandle, MAX_ENTITIES> EntHandleList;

    REFL_ENUM(  EntityType,
                INVALID = 0,
                UNITS_BEGIN,
                UNIT_TEST,
                UNITS_END,

                TYPE_PROP
    );

    enum ENTITY_SELECTION_CHANGE {
        ENTITY_SELECTION_CHANGE_SET,
        ENTITY_SELECTION_CHANGE_ADD,
        ENTITY_SELECTION_CHANGE_REMOVE
    };

    struct Navigator {
        bool                hasDest;
        glm::vec2           dest;

        f32                 slowRad;
    };

    class SpriteResource; // @HACK: SimEntity should not know about this
    struct SpriteAnimator {
        SpriteResource *    sprite;
        i32                 frameIndex;
        f32                 frameTimer;
        f32                 frameDuration;
        i32                 loopCount;
        i32                 frameDelaySkip;
        bool                loops;

        void                SetFrameRate( f32 fps );
        bool                SetSpriteIfDifferent( Core * core, SpriteResource * sprite, bool loops );
        void                Update( Core * core, f32 dt );
        void                TestFrameActuations( Core * core );

        REFLECT();
    };

    struct UnitTurret {
        i32                 idx;
        glm::vec2           pos; // @NOTE: In Local Space
        f32                 ori;
        f32                 fireTimer;
        f32                 fireRate;
        f32                 fireRange;
        f32                 fireDamage;
    };

    struct Unit {
        i32                         maxHealth;
        i32                         currentHealth;

        f32                         averageFiringRange;

        FixedList<UnitTurret, 4>    turrets;
    };

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;

        i32                         playerNumber;
        i32                         teamNumber;

        bool                        active;

        glm::vec2                   pos;
        glm::vec2                   vel;
        glm::vec2                   acc;
        f32                         resistance;
        f32                         ori;

        // Make these flags
        bool                        hasHitCollision;
        bool                        isCollisionStatic;
        Collider2D                  collisionCollider;  // @NOTE: Used for movement | In Local Space

        bool                        isSelectable;
        FixedList<i32, MAX_PLAYERS> selectedBy;
        Collider2D                  selectionCollider;

        Unit                        unit;
        Navigator                   navigator;

        glm::vec2                   visPos;
        f32                         visOri;

        SpriteAnimator              spriteAnimator;
        SpriteAnimator              selectionAnimator;

        Collider2D           GetWorldCollisionCollider() const;
        Collider2D           GetWorldSelectionCollider() const;

        REFLECT();
    };

    class SimMap {
    public:
        bool                                        runSim = false;
        i32                                         mapWidth = 0;
        i32                                         mapHeight = 0;
        LargeString                                 mapName = {};
        FixedObjectPool< SimEntity, MAX_ENTITIES >  entityPool = {};

        f32                                         dtAccumulator = 0.0f;

        FixedList<i32, 4>                           playerNumbers = {};

        MapCommunicator *                           communicator = nullptr;

    public:
        void                                        Initialize( MapCommunicator * communicator, bool runSim );
        void                                        Update( Core * core, f32 dt );
        SimEntity *                                 SpawnEntity( EntityType type, i32 playerNumber, i32 teamNumber, glm::vec2 pos, glm::vec2 vel );

        void                                        Action_Select( i32 playerNumber, EntHandleList & selection, ENTITY_SELECTION_CHANGE change );
        void                                        Action_Move( i32 playerNumber, glm::vec2 pos );
        void                                        Action_Attack( i32 playerNumber, EntityHandle target );

        REFLECT();

    private:
        EntList                                     activeEntities = {};
    };

    struct MapTurn {
        i64 checkSum;
        i32 turnNumber;
    };

    class SyncQueues {
    public:
        void      Start();
        bool      CanTurn();
        void      AddTurn( i32 playerNumber, const MapTurn & turn );
        MapTurn * GetNextTurn( i32 playerNumber );
        void      FinishTurn();
        i32       GetSlidingWindowWidth() { return slidingWindowWidth; }

    private:
        i32 slidingWindowWidth = 4;
        FixedQueue<MapTurn, 10> player1Turns;
        FixedQueue<MapTurn, 10> player2Turns;
    };


    

}

