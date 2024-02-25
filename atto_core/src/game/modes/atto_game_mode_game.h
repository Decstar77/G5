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
        UNIT_STATE_IDLE = 0,
        UNIT_STATE_MOVING = 1,
        UNIT_STATE_ATTACKING = 2,
        UNIT_STATE_TAKING_DAMAGE = 3,
        
    };

    struct UnitStuff {
        UnitState state;
        f32       takingDamageTimer;
    };

    struct SpriteAnimator {
        SpriteResource *    sprite;
        i32                 frameIndex;
        f32                 frameTimer;
        f32                 frameDuration;
        i32                 loopCount;

        inline void SetFrameRate( f32 fps ) { frameDuration = 1.0f / fps; }
        inline void SetSpriteIfDifferent( SpriteResource * sprite ) { 
            if( this->sprite != sprite ) {
                this->sprite = sprite;
                frameIndex = 0;
                frameTimer = 0;
                loopCount = 0;
            }
        }
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

    struct Entity {
        EntityHandle        handle;
        EntityType          type;

        EntityHandle        targetEntity;

        SmallString         name;

        bool                sleeping;

        glm::vec2           pos;
        glm::vec2           vel;
        f32                 ori;

        f32                 facingDir; // 1.0f -> right | -1.0f -> left

        i32                 maxHealth;
        i32                 currentHealth;

        // Make these flags
        bool                hasHitCollision;
        bool                isCollisionStatic;
        bool                isSelectable;

        Collider2D                selectionCollider;
        Collider2D                collisionCollider;  // @NOTE: Used for movement | In Local Space
        FixedList<Collider2D, 8>  hitColliders;       // @NOTE: Used for shooting and stuff | In Local Space

        bool selected;

        i32 teamNumber;

        bool                    wantsDraw;
        SpriteAnimator          spriteAnimator;

        ParticleSystem          particleSystem;

        // The stuffs
        union {
            PlayerStuff         playerStuff;
            UnitStuff           unitStuff;
        };

        inline Collider2D GetWorldCollisionCollider() const;
        inline Collider2D GetWorldSelectionCollider() const;
    };

    class Map {
    public:
        inline static f32 BlockDim = 4.0f;
        i32                                     mapWidth;
        i32                                     mapHeight;

        glm::vec3                               playerStartPos;

        FixedObjectPool< Entity, MAX_ENTITIES > entityPool = {};
        f32                                     dtAccumulator = 0.0f;

        Entity *                                localPlayer = nullptr;

    public:
        // @NOTE: "Map Live" functions 
        void                            Start( Core * core );
        void                            UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags );
        Entity *                        SpawnEntity( EntityType type );
        Entity *                        SpawnDrone( glm::vec2 pos );
        void                            EntityUpdatePlayer( Core * core, Entity * ent, EntList & activeEnts );
    };

    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
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
