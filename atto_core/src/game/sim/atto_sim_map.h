#pragma once

#include "../../shared/atto_core.h"

namespace atto {
    constexpr static i32 MAX_ENTITIES = 1024;
    constexpr static i32 MAX_PLAYERS = 4;

    constexpr i32 TURNS_PER_SECOND = 16;
    constexpr float SIM_DT_FLOAT = 1.0f / TURNS_PER_SECOND;
    constexpr fp SIM_DT = Fp( 1.0f / TURNS_PER_SECOND );
    
    
    //inline static i32 MAX_MAP_SIZE = (i32)glm::sqrt( ToFloat( FP_MAX ) );// @NOTE: This comes to +5000
    inline static i32 MAX_MAP_SIZE = 3000;

    inline i32 SecondsToTurns( i32 s ) {
        return s * TURNS_PER_SECOND;
    }

    class Core;
    class SimMap;
    struct SimEntity;
    class RpcHolder;

    typedef ObjectHandle<SimEntity> EntityHandle;
    typedef FixedList<SimEntity *, MAX_ENTITIES>        EntList;
    typedef FixedList<const SimEntity *, MAX_ENTITIES>  ConstEntList;
    typedef FixedList<SimEntity, MAX_ENTITIES>          EntCacheList;
    typedef FixedList<EntityHandle, MAX_ENTITIES>       EntHandleList;
    typedef FixedObjectPool<SimEntity, MAX_ENTITIES>    EntPool;
    typedef TypeSafeNumber<i32, class PlayerNumberType> PlayerNumber;
    typedef TypeSafeNumber<i32, class TeamNumberType>   TeamNumber;

    REFL_ENUM(  EntityType,
                INVALID = 0,
                UNITS_BEGIN,
                UNIT_WORKER,
                UNIT_TEST,
                UNITS_END,

                BULLETS_BEGIN,
                BULLET_SMOL,
                BULLET_MED,
                BULLETS_END,

                STAR,
                PLANET,

                BUILDING_BEGIN,
                BUILDING_STATION,
                BUILDING_SOLAR_ARRAY,
                BUILDING_COMPUTE,
                BUILDING_END,

                TYPE_PROP
    );

    inline bool IsUnitType( EntityType type ) {
        return type > EntityType::UNITS_BEGIN && type < EntityType::UNITS_END;
    }

    inline bool IsBuildingType( EntityType type ) {
        return type > EntityType::BUILDING_BEGIN && type < EntityType::BUILDING_END;
    }

    enum class EntitySelectionChange : u8 {
        SET,
        ADD,
        REMOVE
    };

    struct Navigator {
        bool                hasDest;
        fp2                 dest;
        fp                  slowRad;
    };

    enum class WeaponSize {
        SMALL,
        MEDIUM,
        LARGE
    };

    struct UnitTurret {
        WeaponSize          size;
        i32                 idx;
        fp2                 posOffset; // @NOTE: In Local Space
        fp                  ori;
        fp                  fireTimer;
        fp                  fireRate;
        fp                  fireRange;
    };

    enum class UnitCommandType {
        IDLE = 0,
        MOVE,
        ATTACK,
        FOLLOW,
        CONTRUCT_BUILDING,
    };

    struct UnitCommand {
        UnitCommandType type;
        fp2             targetPos;
        EntityHandle    targetEnt;
    };

    struct Unit {
        UnitCommand                 command;
        i32                         maxHealth;
        i32                         currentHealth;

        fp                          averageRange;

        FixedList<UnitTurret, 4>    turrets;
    };

    struct Bullet {
        WeaponSize                  size;
        i32                         damage;
        fp                          speed;
        fp                          aliveTimer;
        fp                          aliveTime;
        SpriteResource *            sprVFX_SmallExplody;
    };

    enum class PlanetPlacementType {
        INVALID = 0,
        BLOCKED = 1,
        OPEN = 2,
        // Generic names because each race will have a unique name for each
        CREDIT_GENERATOR,
        ENERGY_GENERATOR,
        COMPUTE_GENERATOR,

        //REPAIR_STATION, // That's a cool idea
    };

    constexpr i32 MAX_PLANET_PLACEMENTS = 5 * 3;
    struct Planet {
        FixedList<PlanetPlacementType, MAX_PLANET_PLACEMENTS> placements;
    };

    struct Building {
        bool isBuilding;
        i32 timeToBuildTurns;
        i32 turn;
        i32 giveEnergyAmount;
    };

    inline FixedList<RpcHolder *, 256>         rpcTable = {};

    enum class MapActionType : u8 {
        NONE = 0,

        // These are all the actions that can be taken by the player and serialized across the network.
        PLAYER_SELECTION,
        SIM_ENTITY_UNIT_COMMAND_MOVE,
        SIM_ENTITY_UNIT_COMMAND_ATTACK,
        SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING,
        SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING,

        // These are all the actions that can be taken by the simulation, they are not serialized across the network. It's expected that the determinism of the simulation will be the same across all clients.
        SIM_ENTITY_SPAWN,
        SIM_ENTITY_DESTROY,

        SIM_ENTITY_APPLY_DAMAGE,
        SIM_ENTITY_APPLY_CONSTRUCTION,

        SIM_MAP_MONIES_GIVE_ENERGY
    };

    class MapActionBuffer {
    public:
        template<typename... _types_>
        inline void AddAction( MapActionType type, _types_... args ) {
            static_assert( is_present<EntityType, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
            static_assert( is_present<EntityType::_enumerated, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
        #if ATTO_GAME_CHECK_RPC_FUNCTION_TYPES
            bool sameParms = rpcTable[ (i32)type ]->AreParamtersTheSame<void, std::add_pointer_t< _types_ >...>();
            AssertMsg( sameParms, "AddAction :: Adding an action with no corrasponding RPC function, most likey the parameters are not the same. ");
        #endif
            data.Write( &type );
            DoSimSerialize( args... );
        }

    public:
        FixedBinaryBlob<256>    data;

    private:
        template< typename _type_ >
        inline void DoSimSerialize( _type_ type ) {
            static_assert( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            data.Write( &type );
        }

        template< typename _type_, typename... _types_ >
        inline void DoSimSerialize( _type_ type, _types_... args ) {
            static_assert( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            data.Write( &type );
            DoSimSerialize( args... );
        }
    };

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;

        PlayerNumber                playerNumber;
        TeamNumber                  teamNumber;

        bool                        active;

        fp2                   pos;
        fp2                   vel;
        fp2                   acc;
        fp                    resistance;
        fp                    ori;

        // Make these flags
        bool                        hasHitCollision;
        bool                        isCollisionStatic;
        Collider2D                  collisionCollider;  // @NOTE: Used for movement | In Local Space

        bool                        isSelectable;
        FixedList<PlayerNumber, MAX_PLAYERS> selectedBy;
        Collider2D                  selectionCollider;

        Unit                        unit;
        Bullet                      bullet;
        Navigator                   navigator;
        Planet                      planet;
        Building                    building;
        MapActionBuffer             actions;

        // ============ Visual stuffies ============ 

        glm::vec2                   visPos;
        f32                         visOri;

        SpriteAnimator              spriteAnimator;
        SpriteAnimator              selectionAnimator;

        Collider2D                  GetWorldCollisionCollider() const;
        Collider2D                  GetWorldSelectionCollider() const;

        REFLECT();
    };

    struct MapTurn {
        i64                     checkSum;
        i32                     turnNumber;
        PlayerNumber            playerNumber;
        MapActionBuffer         actions;
    };

    class SyncQueues {
    public:
        void      Start();
        bool      CanTurn();
        void      AddTurn( PlayerNumber playerNumber, const MapTurn & turn );
        MapTurn * GetNextTurn( PlayerNumber playerNumber );
        void      FinishTurn();
        i32       GetSlidingWindowWidth() { return slidingWindowWidth; }

    private:
        i32                     slidingWindowWidth = 4;
        FixedQueue<MapTurn, 10> player1Turns = {};
        FixedQueue<MapTurn, 10> player2Turns = {};
    };

    struct PlayerMonies {
        i32 credits;
        i32 energy;
        i32 compute;
    };

    class SimMap {
    public:
        Core *                                      core = nullptr;
        i32                                         turnNumber = 0;

        bool                                        runSim = false;
        LargeString                                 mapName = {};
        EntPool                                     entityPool = {};

        f32                                         dtAccumulator = 0.0f;

        i32                                         syncTurnAttempts = {};
        i32                                         syncWaitTurnCounter = {};
        SyncQueues                                  syncQueues = {};
        FixedList<PlayerNumber, 4>                  playerNumbers = {};
        FixedList<PlayerMonies, 4>                  playerMonies = {};

        PlayerNumber                                localPlayerNumber = PlayerNumber::Create( -1 );
        TeamNumber                                  localPlayerTeamNumber = TeamNumber::Create( -1 );
        glm::vec2                                   localCameraPos = glm::vec2( 0.0f );
        i32                                         localCameraZoomIndex = 0;
        glm::vec2                                   localCameraZoomLerp = glm::vec2( 0.0f );
        MapTurn                                     localMapTurn = {};
        MapActionBuffer                             localActionBuffer = {}; // Player inputs
        bool                                        localMapTurnWriting = false;
        bool                                        localIsDragging = false;
        glm::vec2                                   localStartDrag = glm::vec2( 0.0f );
        glm::vec2                                   localEndDrag = glm::vec2( 0.0f );
        EntHandleList                               localNewSelection = {};

        // @HACK:
        bool isPlacingBuilding = false;

    public:
        void                                        Initialize( Core * core );
        void                                        Update( Core * core, f32 dt );
        
        SimEntity *                                 SpawnEntity( EntityType type, PlayerNumber playerNumber, TeamNumber teamNumber, fp2 pos, fp ori, fp2 vel );
        void                                        DestroyEntity( SimEntity * entity );

        void                                        SimTick( MapTurn * turn1, MapTurn * turn2 );
        void                                        Sim_ApplyActions( MapActionBuffer * actionBuffer );

        void                                        SimAction_SpawnEntity( i32 * type, PlayerNumber * playerNumberPtr, TeamNumber * teamNumber, fp2 * pos, fp * ori, fp2 * vel );
        void                                        SimAction_DestroyEntity( EntityHandle * handle );
        void                                        SimAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange * change );
        //void                                        SimAction_PlayerWorldCommand( i32 * playerNumber, glm::vec2 targetPos, EntityHandle * targetEnt ); // Right clicking in the world

        void                                        SimAction_Move( PlayerNumber * playerNumberPtr, fp2 * pos );
        void                                        SimAction_Attack( PlayerNumber * playerNumberPtr, EntityHandle * target );
        void                                        SimAction_ContructBuilding( PlayerNumber * playerNumberPtr, i32 * typePtr, fp2 * posPtr );
        void                                        SimAction_ContructExistingBuilding( PlayerNumber * playerNumberPtr, EntityHandle * target );
        void                                        SimAction_ApplyDamage( i32 * damage, EntityHandle * target );
        void                                        SimAction_ApplyContruction( EntityHandle * target );
        void                                        SimAction_GiveEnergy( PlayerNumber * playerNumberPtr, i32 * amount );


        REFLECT();

    private:
        
        EntList                                     localCacheSelection = {};
        EntList                                     activeEntities = {};
    };

    constexpr size_t SIZE_OF_ENT_LIST = sizeof( EntList );
    constexpr size_t SIZE_OF_ENT_CACHE_LIST = sizeof( EntCacheList );


}
