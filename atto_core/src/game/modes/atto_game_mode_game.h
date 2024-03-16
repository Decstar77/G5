#pragma once

#include "../atto_game.h"
#include "../atto_tiles.h"

namespace atto {
    constexpr static int    MAX_ENTITIES = 1024;

    class Map;
    struct Entity;
    struct MapFileEntity;

    typedef ObjectHandle<Entity> EntityHandle;
    typedef FixedList<Entity *, MAX_ENTITIES> EntList;

    REFL_ENUM( EntityType,
               INVALID = 0,
               PLAYER,

               ENEMIES_START,
               ENEMY_BOT_DRONE,
               ENEMY_BOT_TURRET,
               ENEMIES_END,

               OBJ_TERMINAL,

               BULLET,
               ENEMY_BULLET,

               TYPE_PROP
    );

    inline bool EntityTypeIsEnemy( EntityType type ) {
        return type > EntityType::ENEMIES_START && type < EntityType::ENEMIES_END;
    }


    enum PlayerState {
        PLAYER_STATE_IDLE = 0,
        PLAYER_STATE_MOVING = 1,
        PLAYER_STATE_ATTACKING = 2,
    };

    struct PlayerStuff {
        PlayerState state;

        f32         weaponTimer;
        f32         weaponOri;
        f32         weaponDir;
        glm::vec2   weaponPos;
    };

    enum UnitState {
        UNIT_STATE_IDLE,
        UNIT_STATE_ALERT,
        UNIT_STATE_ATTACKING,
        UNIT_STATE_TAKING_DAMAGE,
        UNIT_STATE_EXPLODING,
        UNIT_STATE_WANDERING,
        UNIT_STATE_SWARM,
    };

    struct UnitStuff {
        UnitState state;
        f32       fireRateTimer;
        f32       takingDamageTimer;
        bool      playedDeathSound;
    };

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

    struct Particle {
        glm::vec2   pos;
        glm::vec2   vel;
        f32         lifeTime;
        f32         scale;
    };

    struct ParticleSystem {
        // Settings
        bool                        oneShot;
        i32                         count;
        f32                         lifeTime;
        f32                         spawnRate;
        f32                         scaleMin;
        f32                         scaleMax;
        glm::vec2                   velMin;
        glm::vec2                   velMax;
        TextureResource *           texture;

        // State
        bool                        emitting;
        f32                         spawnTimer;
        i32                         spawnedCount;
        FixedList<Particle, 128>    particles;
    };

    struct Navigator {
        bool                active;
        glm::vec2           dest;
    };

    struct Entity {
        EntityHandle                handle;
        EntityType                  type;

        Map *                       map;

        SmallString                 name;
        i32                         playerNumber;

        bool                        active;

        glm::vec2                   pos;
        glm::vec2                   vel;
        glm::vec2                   acc;
        f32                         resistance;
        f32                         ori;
        f32                         facingDir; // 1.0f -> right | -1.0f -> left

        i32                         maxHealth;
        i32                         currentHealth;

        bool                        netStreamed;
        bool                        netStreamer;
        glm::vec2                   netVisualPos;
        glm::vec2                   netDesiredVel;
        i32                         netTempId;

        glm::vec4                   colorMultiplier;

        // Make these flags
        bool                        hasHitCollision;
        bool                        isCollisionStatic;
        Collider2D                  collisionCollider;  // @NOTE: Used for movement | In Local Space

    #if ATTO_EDITOR
        bool                        selected;
        bool                        isSelectable;
        Collider2D                  selectionCollider;
    #endif

        SpriteAnimator              spriteAnimator;
        ParticleSystem              particleSystem;
        Navigator                   navigator;

        // The stuffs
        PlayerStuff             playerStuff;
        UnitStuff               unitStuff;

        inline Collider2D       GetWorldCollisionCollider() const;
        inline Collider2D       GetWorldSelectionCollider() const;

        REFLECT();
    };

    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
        i32         otherPlayerNumber;
    };

    struct GameGUI {
        f32             startX;
        f32             startY;
        DrawContext *   drawContext;
        Core *          core;

        void BeginAbilityBar( Core * core, DrawContext * drawContext );
        void EndAbilityBar();
    };


    class Map {
    public:
        /*
            Stored data
        */

        i32                                     mapWidth = 0;
        i32                                     mapHeight = 0;
        LargeString                             mapName = {};
        FixedObjectPool< Entity, MAX_ENTITIES > entityPool = {};
        TileMap                                 tileMap = {};

        i32                                     netTempId = 1;
        FixedList< Entity, 512 >                temporySpawningEntities = {};
        FixedList< Entity, 512 >                temporyDestroyingEntities = {};

        /*
            Runtime data
        */
        f32                                     dtAccumulator = 0.0f;

        glm::vec2                               localCameraPos = glm::vec2( 0.0f );

        bool                                    isStarting = false;
        bool                                    isMp = false;
        bool                                    isAuthority = false;
        

        i32                                     localPlayerNumber = -1;
        i32                                     otherPlayerNumber = -1;

        Entity *                                localPlayer = nullptr;
        Entity *                                otherPlayer = nullptr;
        FixedList< Entity *, 2 >                players = {};
        GameGUI                                 ui = {};

    public:
        // @NOTE: "Map Live" functions 
        void                                    Start( Core * core, const GameStartParams & parms );
        void                                    UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags );

        void                                    SetupEntity( Core * core, Entity * entity, EntityType type, glm::vec2 pos, glm::vec2 vel );

        Entity *                                SpawnEntitySim( Core * core, EntityType type, glm::vec2 pos, glm::vec2 vel );
        Entity *                                SpawnEntityResolve( Core * core, i32 netId );
        Entity *                                SpawnEntity( Core * core, EntityType type, glm::vec2 pos, glm::vec2 vel = glm::vec2( 0, 0 ) );

        void                                    DestroyEntitySim( Core * core, Entity * entity );
        void                                    DestroyEntityResolve( Core * core, i32 netId );
        void                                    DestroyEntity( Core * core, Entity * entity );

        void                                    Unit_Fire( Core * core, Entity * ent, f32 fireRate );
        void                                    Unit_TakeDamageSim( Core * core, Entity * ent, i32 damage );
        void                                    Unit_TakeDamage( Core * core, Entity *ent, i32 damage );

        bool                                    LineOfSite( Entity * a, Entity * b, f32 maxDist );
        Entity *                                ClosestPlayerTo( glm::vec2 p, f32 & dist, glm::vec2 & dir );

    #if ATTO_EDITOR
        // @NOTE: "Map Editor" functions
        void                                    Editor_MapTilePlace( i32 xIndex, i32 yIndex, i32 spriteX, i32 spriteY, i32 flags );
        void                                    Editor_MapTileFillBorder( i32 spriteX, i32 spriteY, i32 flags );

    #endif

        REFLECT();
    };

    class GameMode_Game : public GameMode {
    public:
        GameMode_Game( GameStartParams parms ) : startParms( parms ) {};
        virtual GameModeType            GetGameModeType() override;
        virtual bool                    IsInitialized() override;
        virtual void                    Initialize( Core * core ) override;
        virtual void                    UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) override;
        virtual void                    Shutdown( Core * core ) override;
        virtual Map *               GetMap() override { return &map; }

    public:
        GameStartParams                         startParms;
        Map                                     map;
    };
}
