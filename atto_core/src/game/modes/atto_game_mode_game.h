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

    struct EntityId {
        i32 index;
        i32 generation;
        bool operator==( const EntityId & other ) const {
            return index == other.index && generation == other.generation;
        }
    };

    struct Entity {
        EntityId id;
        EntityType type;
        glm::vec2 pos;
        glm::vec2 velocity;
        f32 rotation;
        TextureResource * sprite;
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
        FixedList<Entity, 1024> entities;
        
    public: // Resources
        TextureResource * sprShipA = nullptr;
        TextureResource * sprShipB = nullptr;
        TextureResource * sprEnemyA = nullptr;
        TextureResource * sprEnemyB = nullptr;
        TextureResource * sprStationA = nullptr;
    };
}
