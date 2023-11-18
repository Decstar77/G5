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
        sprSelectionCircle = core->ResourceGetAndLoadTexture( "unit_selection_01.png" );
    }

    void GameModeGame::Update( Core * core, f32 dt ) {
        glm::vec2 mousePosWorld = core->InputMousePosWorld();

        FixedList<Entity *, MAX_ENTITIES> & selectedEntities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();

        const int entityCount = entities.GetCapcity();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities.Get( entityIndex );
            if( ent->id.index != -1 ) {
                switch( ent->type ) {
                    case ENTITY_TYPE_INVALID: break;
                    case ENTITY_TYPE_SHIP_BEGIN: break;
                    case ENTITY_TYPE_SHIP_A:
                    {
                        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) ) {
                            Circle c = {};
                            c.pos = ent->pos;
                            c.rad = 32.0f;

                            if( c.Contains( mousePosWorld ) == true ) {
                                ent->selected = true;
                            }
                            else {
                                ent->selected = false;
                            }
                        }
                        
                        if( ent->selected ) {
                            selectedEntities.Add( ent );
                        }
                        
                        const f32 acc = 500;
                        const f32 maxSpeed = 200;
                        const f32 maxSpeed2 = maxSpeed * maxSpeed;
                        const f32 drag = 0.98f;

                        if( ent->dest.valid ) {
                            glm::vec2 dir = ent->dest.pos - ent->pos;
                            glm::vec2 ndir = glm::normalize( dir );

                            ent->vel += ndir * acc * dt;
                            if( glm::length2( ent->vel ) > maxSpeed2 ) {
                                ent->vel = glm::normalize( ent->vel );
                                ent->vel = ent->vel * maxSpeed;
                            }

                            if( glm::distance2( ent->pos, ent->dest.pos ) < 10.0f ) {
                                ent->dest.valid = false;
                            }
                        }

                        ent->vel *= drag;

                        ent->pos += ent->vel * dt;

                    } break;
                    case ENTITY_TYPE_SHIP_END: break;
                    case ENTITY_TYPE_ENEMY: break;
                    default: break;
                }
            }
        }

        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) ) {
            ShipDestination dest = {};
            dest.pos = mousePosWorld;
            dest.valid = true;
            
            const int selectedEntityCount = selectedEntities.GetCount();
            for( int selectedEntityIndex = 0; selectedEntityIndex < selectedEntityCount; ++selectedEntityIndex ) {
                Entity * ent = selectedEntities[selectedEntityIndex];
                ent->dest = dest;
            }
        }
    }

    void GameModeGame::Render( Core * core, f32 dt ) {
        const int entityCount = entities.GetCapcity();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities.Get( entityIndex );
            if( ent->id.index != -1 ) {
                switch( ent->type ) {
                    case ENTITY_TYPE_INVALID: break;
                    case ENTITY_TYPE_SHIP_BEGIN: break;
                    case ENTITY_TYPE_SHIP_A:
                    {
                        if( ent->selected ) {
                            core->RenderDrawSprite( sprSelectionCircle, ent->pos );
                        }

                        core->RenderDrawSprite( sprEnemyA, ent->pos, 0.0f, glm::vec2( 1 ) );
                        //core->RenderDrawRect( ent->pos, glm::vec2( 32 ), 0.0f );

                    } break;
                    case ENTITY_TYPE_SHIP_END: break;
                    case ENTITY_TYPE_ENEMY: break;
                    default: break;
                }
            }
        }

        FontHandle f = core->ResourceGetFont("default");
        core->RenderDrawText( f, glm::vec2( 0, 0 ), "Hello World" );

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
        Entity * ent = SpawnEntity( type );
        if( ent != nullptr ) {
            ent->pos = pos;
        }

        return ent;
    }

}
