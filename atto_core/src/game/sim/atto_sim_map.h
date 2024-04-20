#pragma once

#include "atto_sim_base.h"
#include "atto_sim_sync_queues.h"
#include "../../shared/atto_ui.h"

namespace atto {
    class Core;
    class RpcHolder;


    inline bool IsUnitType( EntityType type ) { return type > EntityType::UNITS_BEGIN && type < EntityType::UNITS_END; }
    inline bool IsBuildingType( EntityType type ) { return type > EntityType::BUILDING_BEGIN && type < EntityType::BUILDING_END; }
    inline bool IsWorkerType( EntityType type ) { return type == EntityType::UNIT_KLAED_WORKER || type == EntityType::UNIT_NAUTOLAN_WORKER || type == EntityType::UNIT_NAIRAN_WORKER; }

    i32                 GetUnitTrainTimeForEntityType( EntityType type );
    MoneySet            GetUnitCostForEntityType( EntityType type );
    char                GetUnitSymbolForEntityType( EntityType type );
    SpriteResource *    GetSpriteForBuildingType( EntityType type );
    MoneySet            GetBuildingCostForEntityType( EntityType type );
    FpCollider          GetBaseColliderForBuildingType( EntityType type );

    enum class NavigatorFacingMode {
        VEL = 0,
        TARGET,
    };

    struct Navigator {
        bool                hasDest;
        fp2                 dest;
        fp                  slowRad;
        NavigatorFacingMode facingMode;
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

    struct UnitWeapon {
        fp2                         posOffset;
        i32                         fireRateDelayTurns;
        i32                         fireTimerDelayTurns;
        bool                        hasFired;
    };

    struct Unit {
        UnitCommand                 command;
        fp                          range;
        fp                          speed;
        i32                         fireTimerTurns;
        i32                         fireRateTurns;
        EntityType                  weaponType;
        FixedList<UnitWeapon, 6>    weapons;
    };

    struct Bullet {
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
        FixedQueue< EntityType, 64 > trainingQueue;

        i32 timeToGiveEnergyTurns;
        i32 amountToGiveEnergy;
        i32 timeToGiveComputeTurns;
        i32 amountToGiveCompute;
        i32 timeToGiveCreditsTurns;
        i32 amountToGiveCredits;
    };

    struct SolarSystemConnectionPair {
        EntityHandle a;
        EntityHandle b;

        inline bool operator==( const SolarSystemConnectionPair & other ) { return a == other.a && b == other.b; }
    };

    struct SimEntitySpawnInfo {
        EntityType type;
        PlayerNumber playerNumber;
        TeamNumber  teamNumber;
        SolarNumber solarNumber;
        fp2 pos;
        fp2 vel;
        fp ori;
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
        FpCollider                  collisionCollider;  // @NOTE: Used for movement | In Local Space

        bool                        isSelectable;
        FixedList<PlayerNumber, MAX_PLAYERS> selectedBy; // @TODO: Could be optimized to be a espcially if player number is bit 8, could store all of the state in i32 or i64 for 8 playes

        i32                         maxHealth;
        i32                         currentHealth;

        union {
            Unit                        unit;
            Planet                      planet;
            Building                    building;
            Bullet                      bullet;
        };

        Navigator                   navigator;
        MapActionBuffer             actions;

        // ============ Visual stuffies ============ 
        glm::vec2                               visPos;
        f32                                     visOri;
        bool                                    visHasDest;
        glm::vec2                               visDestPos;
        FixedList<PlayerNumber, MAX_PLAYERS>    visSelectedBy;
        Collider2D                              visSelectionCollider;

        SpriteAnimator                          spriteAnimator;
        SpriteAnimator                          engineAnimator;
        SpriteAnimator                          shieldAnimator;
        SpriteAnimator                          selectionAnimator;
        union {
            SpriteResource * spriteBank[8];
            struct {
                SpriteResource * base;
                SpriteResource * engine;
                SpriteResource * shield;
                SpriteResource * weapons;
                SpriteResource * destruction;
            } spriteUnit;
            struct {
                SpriteResource * base;
                SpriteResource * destruction;
            } sprBullet;
        };

        AudioResource *                         sndHello;
        AudioResource *                         sndMove;
        FixedList<AudioResource *, 4>           sndDestructions;

        FpCollider                  GetWorldCollisionCollider() const;
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
        EntityListFilter *  Types( EntityType * types, i32 count );
        EntityListFilter *  TypeRange( EntityType::_enumerated start, EntityType::_enumerated end );
        EntityListFilter *  End();

        bool                ContainsOnlyWorker();
        bool                ContainsOnlyType( EntityType::_enumerated type );
        bool                ContainsOnlyOneOfType( EntityType::_enumerated type );

    private:
        EntList *                       activeEntities;
        FixedList<bool, MAX_ENTITIES>   marks;
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

    struct Particle {
        glm::vec2 pos;
        f32 ori;
        bool alive;
        SpriteAnimator spriteAnimator;
    };

    class AIThinker {
    public:
        bool                clientHasAuth = false;
        PlayerNumber        aiNumber = {};
        MapActionBuffer     actionBuffer = {};
        SimMap *            map = nullptr;
        Core *              core = nullptr;
        EntHandleList       selection = {};
        i32                 nextThinkTurn = 0;
        i32                 lastBuildingTurn = 0;


        void                Think( EntList & activeEntities );
        bool                FindBuildingLocation( EntList & activeEntities, EntityType buildingType, fp2 basePos, fp2 & pos );
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
        FixedList< PlayerNumber, MAX_PLAYERS >      playerNumbers = {};
        FixedList< MoneySet, MAX_PLAYERS >          playerMonies = {};

        AIThinker                                   aiThinker = {};

        // ================= Other =================
        SimMapReplay                                mapReplay = {};

        // ============ Visual Stuffies ============
        bool                                        viewIsDragging = false;
        glm::vec2                                   viewStartDrag = glm::vec2( 0.0f );
        glm::vec2                                   viewEndDrag = glm::vec2( 0.0f );
        EntHandleList                               viewDragSelection = {};
        SolarNumber                                 viewSolarNumber = {};
        FixedList<SolarSystemConnectionPair, 1000>  viewSolarSystemConnections = {};
        i32                                         viewParticleAliveCount = 0;
        FixedList< Particle, 1000 >                 viewParticles = {};

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
        bool                    SimUtil_CanPlaceBuilding( EntityListFilter * entityFilter, EntList & entities, FpCollider collider );

        void                    SimAction_Pay( PlayerNumber playerNumber, MoneySet costSet );
        void                    SimAction_SpawnEntity( i32 type, PlayerNumber playerNumberPtr, TeamNumber teamNumber, SolarNumber solarNumberPtr, fp2 pos, fp ori, fp2  vel );
        void                    SimAction_DestroyEntity( EntityHandle handle );
        void                    SimAction_PlayerSelect( PlayerNumber playerNumberPtr, Span<EntityHandle> selection, EntitySelectionChange change );
        void                    SimAction_Move( PlayerNumber playerNumberPtr, fp2 pos );
        void                    SimAction_Attack( PlayerNumber playerNumberPtr, EntityHandle target );
        void                    SimAction_ContructBuilding( PlayerNumber playerNumberPtr, i32 typePtr, fp2 posPtr );
        void                    SimAction_ContructExistingBuilding( PlayerNumber playerNumberPtr, EntityHandle target );
        void                    SimAction_PlanetPlacePlacement( PlayerNumber playerNumberPtr, i32 placementIndexPtr, PlanetPlacementType placementTypePtr );
        void                    SimAction_BuildingTrainUnit( PlayerNumber playerNumberPtr , i32 typePtr );
        void                    SimAction_BuildingCancelUnit( PlayerNumber playerNumberPtr , i32 indexPtr );
        void                    SimAction_ApplyDamage( i32 damage, EntityHandle target );
        void                    SimAction_ApplyContruction( EntityHandle target );
        void                    SimAction_GiveCredits( PlayerNumber playerNumberPtr, i32 amountPtr );
        void                    SimAction_GiveEnergy( PlayerNumber playerNumberPtr, i32 amountPtr );
        void                    SimAction_GiveCompute( PlayerNumber playerNumberPtr, i32 amountPtr );
        void                    SimAction_Test( Span<i32> test );

        void                    VisAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange change );

        REFLECT();

    private:
        EntList                                     simAction_ActiveEntities = {};
        EntityListFilter                            simAction_EntityFilter = {};
    };
}

