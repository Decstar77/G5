#pragma once

#include "atto_sim_base.h"
#include "atto_sim_sync_queues.h"

namespace atto {
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
                SOLAR_SYSTEM,

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

    inline static const char * EntitySelectionChangeStrings[] = { 
        "SET",
        "ADD",
        "REMOVE"
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

        fp                          averageRange;

        FixedList<UnitTurret, 4>    turrets;
    };

    struct Bullet {
        WeaponSize                  size;
        i32                         damage;
        fp                          speed;
        fp                          aliveTimer;
        fp                          aliveTime;
    };

    enum class PlanetPlacementType : u8 {
        INVALID = 0,
        BLOCKED = 1,
        OPEN = 2,
        // Generic names because each race will have a unique name for each
        CREDIT_GENERATOR,
        ENERGY_GENERATOR,
        COMPUTE_GENERATOR,

        //REPAIR_STATION, // That's a cool idea
    };

    inline static const char * PlanetPlacementTypeStrings[] = {
        "INVALID",
        "BLOCKED",
        "OPEN",
        "CREDIT_GENERATOR",
        "ENERGY_GENERATOR",
        "COMPUTE_GENERATOR"
    };

    constexpr i32 MAX_PLANET_PLACEMENTS = 5 * 3;
    struct Planet {
        FixedList<PlanetPlacementType, MAX_PLANET_PLACEMENTS> placements;
        FixedList<i32, MAX_PLANET_PLACEMENTS>                 placementsTurns;
    };

    struct Building {
        i32 turn;
        bool isBuilding;
        i32 timeToBuildTurns;
        bool isTraining;
        i32 timeToTrainTurns;
        EntityType trainingEnt;

        i32 timeToGiveEnergyTurns;
        i32 amountToGiveEnergy;
        i32 timeToGiveComputeTurns;
        i32 amountToGiveCompute;
    };

    struct SolarSystemConnectionPair {
        EntityHandle a;
        EntityHandle b;

        inline bool operator==( const SolarSystemConnectionPair & other ) { return a == other.a && b == other.b; }
    };

    struct SolarSystem {
        SmallString                 name;
        FixedList<EntityHandle, 8>  connections;
    };

    struct SimEntitySpawnInfo {
        EntityType type;
        PlayerNumber playerNumber;
        TeamNumber  teamNumber;
        SolarNumber solarNumber;
        fp2 pos;
        fp2 vel;
        fp ori;

        union {
            SolarSystem solarSystem;
        };
    };

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;

        PlayerNumber                playerNumber;
        TeamNumber                  teamNumber;
        SolarNumber                 solarNumber;

        bool                        active;

        fp2                         pos;
        fp2                         vel;
        fp2                         acc;
        fp                          resistance;
        fp                          ori;

        // Make these flags
        bool                        hasHitCollision;
        bool                        isCollisionStatic;
        Collider2D                  collisionCollider;  // @NOTE: Used for movement | In Local Space

        bool                        isSelectable;
        FixedList<PlayerNumber, MAX_PLAYERS> selectedBy; // @TODO: Could be optimized to be a espcially if player number is bit 8, could store all of the state in i32 or i64 for 8 playes
        Collider2D                  selectionCollider;

        i32                         maxHealth;
        i32                         currentHealth;

        Unit                        unit;
        Bullet                      bullet;
        Navigator                   navigator;
        Planet                      planet;
        Building                    building;
        SolarSystem                 solarSystem;
        MapActionBuffer             actions;

        // ============ Visual stuffies ============ 
        glm::vec2                               visPos;
        f32                                     visOri;
        FixedList<PlayerNumber, MAX_PLAYERS>    visSelectedBy;

        SpriteAnimator                          spriteAnimator;
        SpriteAnimator                          engineAnimator;
        SpriteAnimator                          shieldAnimator;
        SpriteAnimator                          selectionAnimator;
        union {
            SpriteResource * spriteBank[8];
            struct {
                SpriteResource * base;
                SpriteResource * engine;
            } spriteUnit;
            struct {
                SpriteResource * base;
                SpriteResource * exploding;
            } sprBullet;
        } ;
        AudioResource *                         sndHello;
        AudioResource *                         sndMove;

        Collider2D                  GetWorldCollisionCollider() const;
        Collider2D                  GetWorldSelectionCollider() const;

        REFLECT();
    };

    class EntityListFilter {
    public:
        EntList     result;

        EntityListFilter *  Begin( EntList * activeEntities );
        EntityListFilter *  OwnedBy( PlayerNumber playerNumber );
        EntityListFilter *  SimSelectedBy( PlayerNumber playerNumber );
        EntityListFilter *  VisSelectedBy( PlayerNumber playerNumber );
        EntityListFilter *  Type( EntityType::_enumerated type );
        EntityListFilter *  IsTypeRange( EntityType::_enumerated start, EntityType::_enumerated end );
        EntityListFilter *  End();

        bool                ContainsOnlyType( EntityType::_enumerated type );
        bool                ContainsOnlyOneOfType( EntityType::_enumerated type );

    private:
        EntList *                       activeEntities;
        FixedList<bool, MAX_ENTITIES>   marks;
    };

    struct MoneySet {
        i32 credits;
        i32 energy;
        i32 compute;
    };

    class SimMapReplay {
    public:
        void Prepare();
        void NextTurn( i32 turn );
        void AddActionData( MapActionBuffer * actionBuffer );
        void PrintActions( Core * core );

    private:
        struct TurnAction {
            i32 turnNumber;
            i32 actionCount;
        };
        std::vector<TurnAction> turns;
        BinaryBlob actionData;
    };

    enum class ViewMode {
        SOLAR,
        GALAXY
    };

    class SimMap {
    public:
        Core *                                      core = nullptr;
        i32                                         turnNumber = 0;

        bool                                        runSim = false;
        LargeString                                 mapName = {};
        EntPool                                     entityPool = {};

        f32                                         dtAccumulator = 0.0f;

        // ============  Local Player   ============
        PlayerNumber                                localPlayerNumber = PlayerNumber::Create( -1 );
        TeamNumber                                  localPlayerTeamNumber = TeamNumber::Create( -1 );
        glm::vec2                                   localCameraPos = glm::vec2( 0.0f );
        i32                                         localCameraZoomIndex = 0;
        glm::vec2                                   localCameraZoomLerp = glm::vec2( 0.0f );
        MapTurn                                     localMapTurn = {};
        MapActionBuffer                             localActionBuffer = {}; // Player inputs

        // ============ Syncing Stuffies ============
        i32                                         syncTurnAttempts = {};
        i32                                         syncWaitTurnCounter = {};
        SyncQueues                                  syncQueues = {};

        // ============ Player Stuffies ============
        FixedList< PlayerNumber, 4 >                playerNumbers = {};
        FixedList< MoneySet, 4 >                playerMonies = {};

        // ================= Other =================
        SimMapReplay                                mapReplay = {};

        // ============ Visual Stuffies ============
        ViewMode                                    viewMode = ViewMode::SOLAR;
        bool                                        viewIsDragging = false;
        glm::vec2                                   viewStartDrag = glm::vec2( 0.0f );
        glm::vec2                                   viewEndDrag = glm::vec2( 0.0f );
        EntHandleList                               viewDragSelection = {};
        SolarNumber                                 viewSolarNumber = {};
        FixedList<SolarSystemConnectionPair, 1000>  viewSolarSystemConnections = {};

        UIContext                                   gameUI = {};

        // @HACK:
        bool isPlacingBuilding = false;
        EntityType placingBuildingType = {};
        bool planetPlacementSubMenu = false;
        i32 planetPlacementSubMenuIndex = -1;
        bool playSpawningSounds = false;

    public:
        void                    Initialize( Core * core );
        void                    Update( Core * core, f32 dt );
        
        SimEntity *             SpawnEntity( SimEntitySpawnInfo * spawnInfo );
        SimEntity *             SpawnEntity( EntityType type, PlayerNumber playerNumber, TeamNumber teamNumber, SolarNumber solarNumber, fp2 pos, fp ori, fp2 vel );
        void                    DestroyEntity( SimEntity * entity );
                                
        void                    SimTick( MapTurn * turn1, MapTurn * turn2 );
        void                    Sim_ApplyActions( MapActionBuffer * actionBuffer );
        i64                     Sim_CheckSum();

        bool                    Vis_CanAfford( PlayerNumber playerNumber, MoneySet costSet );

        bool                    SimUtil_CanAfford( PlayerNumber playerNumber, MoneySet costSet );
        void                    SimUtil_Pay( PlayerNumber playerNumber, MoneySet costSet );
        void                    SimAction_SpawnEntity( i32 * type, PlayerNumber * playerNumberPtr, TeamNumber * teamNumber, SolarNumber * solarNumberPtr, fp2 * pos, fp * ori, fp2 * vel );
        void                    SimAction_DestroyEntity( EntityHandle * handle );
        void                    SimAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange * change );
        void                    SimAction_Move( PlayerNumber * playerNumberPtr, fp2 * pos );
        void                    SimAction_Attack( PlayerNumber * playerNumberPtr, EntityHandle * target );
        void                    SimAction_ContructBuilding( PlayerNumber * playerNumberPtr, i32 * typePtr, fp2 * posPtr );
        void                    SimAction_ContructExistingBuilding( PlayerNumber * playerNumberPtr, EntityHandle * target );
        void                    SimAction_PlanetPlacePlacement( PlayerNumber * playerNumberPtr, i32 * placementIndexPtr, PlanetPlacementType * placementTypePtr );
        void                    SimAction_BuildingTrainUnit( PlayerNumber * playerNumberPtr , i32 * typePtr );
        void                    SimAction_ApplyDamage( i32 * damage, EntityHandle * target );
        void                    SimAction_ApplyContruction( EntityHandle * target );
        void                    SimAction_GiveCredits( PlayerNumber * playerNumberPtr, i32 * amountPtr );
        void                    SimAction_GiveEnergy( PlayerNumber * playerNumberPtr, i32 * amountPtr );
        void                    SimAction_GiveCompute( PlayerNumber * playerNumberPtr, i32 * amountPtr );

        void                    VisAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange change );

        REFLECT();

    private:
        EntList                                     simAction_ActiveEntities = {};
        EntityListFilter                            simAction_EntityFilter = {};
    };
}

