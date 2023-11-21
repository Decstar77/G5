#include "atto_game_mode_game.h"

namespace atto {
    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
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

        FixedList<Entity *, MAX_ENTITIES> & entities = * core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();
        entityPool.GatherActiveObjs( entities );

        FixedList<Entity *, MAX_ENTITIES> & selectedEntities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities[ entityIndex ];
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
                    const f32 drag = 0.90f;
                    glm::vec2 steer = glm::vec2( 0, 0 );

                    const f32 targetRad = 32.0f;    core->RenderDrawCircle( ent->dest.pos, targetRad, glm::vec4( 0, 1, 0, 0.8f ) );
                    const f32 slowRad = 128.0f;     core->RenderDrawCircle( ent->dest.pos, slowRad, glm::vec4( 1, 0, 0, 0.8f ) );
                    const f32 timeToTarget = 0.1f;

                    glm::vec2 dir = ent->dest.pos - ent->pos;
                    f32 dist = glm::length( dir );

                    if( dist < targetRad ) {
                        ent->dest.valid = false;
                    }

                    if( ent->dest.valid ) {
                        f32 targetSpeed = 0.0;
                        if( dist > slowRad ) {
                            targetSpeed = acc;
                        }
                        else {
                            targetSpeed = maxSpeed * ( dist / slowRad );
                        }

                        glm::vec2 targetVel = glm::normalize( dir ) * targetSpeed;
                        steer = targetVel - ent->vel;
                        steer /= timeToTarget;
                    }

                    ent->vel += steer * dt;
                    if( ent->dest.valid == false ) {
                        ent->vel *= drag;
                    }

                    if( glm::length2( ent->vel ) > maxSpeed2 ) {
                        ent->vel = glm::normalize( ent->vel );
                        ent->vel = ent->vel * maxSpeed;
                    }

                    ent->pos += ent->vel * dt;

                } break;
                case ENTITY_TYPE_SHIP_END: break;
                case ENTITY_TYPE_ENEMY: break;
                default: break;
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

        FixedList<Entity *, MAX_ENTITIES> & entities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();
        entityPool.GatherActiveObjs( entities );

        const int entityCount = entities.GetCount();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities[ entityIndex ];
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

        FontHandle f = core->ResourceGetFont("default");
        core->RenderDrawText( f, glm::vec2( 100, 100 ), 128.0f, "Hello World" );

        core->RenderSubmit();
    }

    void GameModeGame::Shutdown( Core * core ) {
    }

    Entity * GameModeGame::SpawnEntity( EntityType type ) {
        EntityHandle hdl = {};
        Entity * entity = entityPool.Add( hdl );
        ZeroStructPtr( entity );
        if( entity != nullptr ) {
            entity->id = hdl;
            entity->type = type;
            entity->selected = false;
            entity->dest.valid = false;
            return entity;
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
