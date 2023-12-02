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


    struct ShipDestination {
        glm::vec2 pos;
        bool valid;
    };
    
    struct Entity;
    typedef ObjectHandle<Entity> EntityHandle;
    struct Entity {
        EntityHandle id;
        EntityType type;
        glm::vec2 pos;
        glm::vec2 vel;
        f32 rotation;
        TextureResource * sprite;
        ShipDestination dest;
        bool selected;
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

    public:
        constexpr static int    MAX_ENTITIES = 1024;
        FixedObjectPool<Entity, MAX_ENTITIES> entityPool;

        bool                                    selectionDragging = false;
        glm::vec2                               selectionStartDragPos = glm::vec2( 0 );
        glm::vec2                               selectionEndDragPos = glm::vec2( 0 );
        
    public: // Resources
        TextureResource * sprShipA = nullptr;
        TextureResource * sprShipB = nullptr;
        TextureResource * sprEnemyA = nullptr;
        TextureResource * sprEnemyB = nullptr;
        TextureResource * sprStationA = nullptr;
        TextureResource * sprSelectionCircle = nullptr;
    };
}
