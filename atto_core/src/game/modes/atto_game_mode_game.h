#pragma once

#include "../atto_game.h"

namespace atto {
    constexpr static int    MAX_ENTITIES = 1024;

    class Map;
    struct Entity;
    enum EntityType;
    struct MapFileEntity;

    typedef ObjectHandle<Entity> EntityHandle;
    typedef FixedList<Entity *, MAX_ENTITIES> EntList;

    enum EntityType {
        ENTITY_TYPE_INVALID = 0,
        ENTITY_TYPE_PLAYER = 1,
        
        ENTITY_TYPE_ENEMIES_START,
        ENTITY_TYPE_ENEMY_DRONE_01,
        
    };

    enum AbilityType {
        ABILITY_TYPE_INVALID = 0,
        ABILITY_TYPE_WARRIOR_STRIKE = 1,
        ABILITY_TYPE_WARRIOR_STAB = 2,
        ABILITY_TYPE_WARRIOR_CHARGE = 3,
    };
    
    struct Ability {
        AbilityType type;
        f32 cooldown;
        f32 range;
        f32 cooldownTimer;
        bool stopsMovement;
        SpriteResource * sprite;

        FixedList< EntityHandle, 32 > hits;
    };

#define MAX_ABILITIES 12

    enum PlayerState {
        PLAYER_STATE_IDLE = 0,
        PLAYER_STATE_MOVING = 1,
        PLAYER_STATE_ATTACKING = 2,
    };

    struct PlayerStuff {
        PlayerState state;
        Ability *   currentAbility;
        Ability *   primingAbility;

        f32 speed;

        union {
            Ability abilities[ MAX_ABILITIES ];
            struct {
                Ability slash;
                Ability strike;
                Ability charge;
            } warrior;
        };
    };

    enum UnitState {
        UNIT_STATE_IDLE,
        UNIT_STATE_ATTACKING,
        UNIT_STATE_TAKING_DAMAGE,
        UNIT_STATE_EXPLODING,
        UNIT_STATE_WANDERING,
        UNIT_STATE_SWARM,
    };

    struct UnitStuff {
        UnitState state;
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
        void                SetSpriteIfDifferent( SpriteResource * sprite, bool loops );
    };

    struct Particle {
        glm::vec2   pos;
        glm::vec2   vel;
        f32         lifeTime;
        f32         scale;
    };

    struct ParticleSystem {
        // Settings
        bool                    oneShot;
        i32                     count;
        f32                     lifeTime;
        f32                     spawnRate;
        f32                     scaleMin;
        f32                     scaleMax;
        glm::vec2               velMin;
        glm::vec2               velMax;
        TextureResource *       texture;

        // State
        bool                      emitting;
        f32                       spawnTimer;
        i32                       spawnedCount;
        FixedList<Particle, 128>  particles;
    };

    struct Navigator {
        bool                active;
        glm::vec2           dest;
    };

    struct Entity {
        EntityHandle        handle;
        EntityType          type;

        SmallString         name;
        i32                 playerNumber;

        glm::vec2           pos;
        glm::vec2           vel;
        f32                 ori;
        f32                 facingDir; // 1.0f -> right | -1.0f -> left

        i32                 maxHealth;
        i32                 currentHealth;

        bool                netStreamed;
        glm::vec2           netDesiredPos;
        glm::vec2           netDesiredVel;


        // Make these flags
        bool                hasHitCollision;
        bool                isCollisionStatic;
        bool                isSelectable;

        Collider2D                selectionCollider;
        Collider2D                collisionCollider;  // @NOTE: Used for movement | In Local Space
        FixedList<Collider2D, 8>  hitColliders;       // @NOTE: Used for shooting and stuff | In Local Space

        bool                    wantsDraw;
        SpriteAnimator          spriteAnimator;
        ParticleSystem          particleSystem;
        Navigator               navigator;

        // The stuffs
        union {
            PlayerStuff         playerStuff;
            UnitStuff           unitStuff;
        };

        inline Collider2D GetWorldCollisionCollider() const;
        inline Collider2D GetWorldSelectionCollider() const;
    };

    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
        i32         otherPlayerNumber;
    };
    
    class Map {
    public:
        inline static f32 BlockDim = 4.0f;
        i32                                     mapWidth = 0;
        i32                                     mapHeight = 0;

        glm::vec3                               playerStartPos = {};

        FixedObjectPool< Entity, MAX_ENTITIES > entityPool = {};
        f32                                     dtAccumulator = 0.0f;

        bool                                    isMp = false;
        i32                                     localPlayerNumber = -1;
        Entity *                                localPlayer = nullptr;
        glm::vec2                               localCameraPos = glm::vec2( 0.0f );
        
    public:
        // @NOTE: "Map Live" functions 
        void                            Start( Core * core, const GameStartParams & parms );
        void                            UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags );
        Entity *                        SpawnEntity( EntityType type );
        Entity *                        SpawnDrone( glm::vec2 pos );
        void                            EntityUpdatePlayer( Core * core, Entity * ent, EntList & activeEnts );
    };

    class GameMode_Game : public GameMode {
    public:
                                        GameMode_Game( GameStartParams parms ) : startParms( parms ) {};
        virtual GameModeType            GetGameModeType() override;
        virtual bool                    IsInitialized() override;
        virtual void                    Initialize( Core * core ) override;
        virtual void                    UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) override;
        virtual void                    Shutdown( Core * core ) override;

    public:
        GameStartParams                         startParms;
        Map                                     map;
    };
}
