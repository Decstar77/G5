#include "atto_game_mode_game.h"

namespace atto {
    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
        const int entityCount = entities.GetCapcity();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * e = entities.Get( entityIndex );
            e->id.index = -1;
            e->id.generation = 0;
        }

        SpawnEntity( ENTITY_TYPE_SHIP_A, glm::vec2( 400, 400 ) );

        sprShipA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/ship_A.png" );
        sprShipB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/ship_B.png" );
        sprEnemyA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_A.png" );
        sprEnemyB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_B.png" );
        sprStationA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/station_A.png" );
    }

    void GameModeGame::Update( Core * core, f32 dt ) {

    }

    void GameModeGame::Render( Core * core, f32 dt ) {
        const int entityCount = entities.GetCapcity();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities.Get( entityIndex );
            if( ent->id.index != -1 ) {
                switch( ent->type ) {
                    case ENTITY_TYPE_INVALID: break;
                    case ENTITY_TYPE_SHIP_BEGIN: break;
                    case ENTITY_TYPE_SHIP_A: {
                        core->RenderDrawSprite( sprShipA, ent->pos, 0.0f, glm::vec2( 2 ) );
                        //core->RenderDrawRect( ent->pos, glm::vec2( 32 ), 0.0f );
                    } break;
                    case ENTITY_TYPE_SHIP_END: break;
                    case ENTITY_TYPE_ENEMY: break;
                    default: break;
                }
            }
        }

        
        core->RenderSubmit();
    }

    void GameModeGame::Shutdown( Core * core ) {
    }

    Entity * GameModeGame::SpawnEntity( EntityType type ) {
        const int entityCount = entities.GetCapcity();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities.Get( entityIndex );
            if( ent->id.index == -1 ) {
                ent->id.index = entityIndex;
                ent->id.generation++;
                ent->type = type;
                return ent;
            }
        }

        Assert( false );

        return nullptr;
    }

    Entity * GameModeGame::SpawnEntity( EntityType type, glm::vec2 pos ) {
        Entity *ent = SpawnEntity( type );
        if( ent != nullptr ) {
            ent->pos = pos;
        }

        return ent;
    }

}
