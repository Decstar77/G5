#pragma once

#include "../atto_game.h"

namespace atto {
    enum EntityType {
        ENTITY_TYPE_INVALID = 0,
        ENTITY_TYPE_SHIP_BEGIN,
        ENTITY_TYPE_SHIP_A,
        ENTITY_TYPE_SHIP_END,

        ENTITY_TYPE_ENEMY,
    };

    struct Entity;
    typedef ObjectHandle<Entity> EntityHandle;
    constexpr static int    MAX_ENTITIES = 1024;

    struct ArrivalCircle;
    typedef ObjectHandle<ArrivalCircle> ArrivalCircleHandle;
    struct ArrivalCircle {
        ArrivalCircleHandle id;
        FixedList<EntityHandle, MAX_ENTITIES> ents;
        i32 aliveHandleCount;
        glm::vec2 pos;
        f32 targetRad;
        f32 slowRad;
        f32 timeToTarget;
    };

    struct ShipDestination {
        ArrivalCircleHandle arrivalCircle;
        bool moving;
    };

    
    struct Entity {
        EntityHandle id;
        EntityType type;
        glm::vec2 pos;
        glm::vec2 vel;
        f32 ori;
        TextureResource * sprite;
        ShipDestination dest;
        bool hasCollision;
        Circle collisionCircle;
        bool selected;

        
        inline Circle GetCollisionCircleWorld() const {
            Circle c = collisionCircle;
            c.pos += pos;
            return c;
        }
    };

    struct SprTileSheet {
        TextureResource * tile01;
    };

    class GameModeGame : public GameMode {
    public:
        GameModeType GetGameModeType() override;
        void Init( Core * core ) override;
        void Update( Core * core, f32 dt ) override;
        void Render( Core * core, f32 dt ) override;
        void Shutdown( Core * core ) override;

    public:
        Entity * SpawnEntity( EntityType type );
        Entity * SpawnEntity( EntityType type, glm::vec2 pos );
        Entity * SpawnEntityShipA( glm::vec2 pos );

    public:
        FixedObjectPool<Entity, MAX_ENTITIES> entityPool;

        bool                                    selectionDragging = false;
        glm::vec2                               selectionStartDragPos = glm::vec2( 0 );
        glm::vec2                               selectionEndDragPos = glm::vec2( 0 );
        
        FixedObjectPool<ArrivalCircle, MAX_ENTITIES> arrivalCirclePool;

    public: 
        // Resources
        TextureResource * spr_RedWorker = nullptr;
        TextureResource * sprShipB = nullptr;
        TextureResource * sprEnemyA = nullptr;
        TextureResource * sprEnemyB = nullptr;
        TextureResource * sprStationA = nullptr;
        TextureResource * sprSelectionCircle = nullptr;
        SprTileSheet      sprTileSheet = {};
    };
}
