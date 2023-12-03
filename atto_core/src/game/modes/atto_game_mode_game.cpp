#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
        SpawnEntityShipA( glm::vec2( 400, 400 ) );
        SpawnEntityShipA( glm::vec2( 200, 400 ) );

        SpawnEntityShipA( glm::vec2( 400, 600 ) );
        SpawnEntityShipA( glm::vec2( 200, 600 ) );

        SpawnEntityShipA( glm::vec2( 400, 200 ) );
        SpawnEntityShipA( glm::vec2( 200, 200 ) );

        spr_RedWorker = core->ResourceGetAndLoadTexture( "kenny_sprites_03/unit/scifiUnit_11.png" );
        sprShipB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/ship_B.png" );
        sprEnemyA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_A.png" );
        sprEnemyB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_B.png" );
        sprStationA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/station_A.png" );
        sprSelectionCircle = core->ResourceGetAndLoadTexture( "unit_selection_01.png" );
    }

    void GameModeGame::Update( Core * core, f32 dt ) {
        const bool mouseHasMoved = core->InputMouseHasMoved();
        const glm::vec2 mousePosWorld = core->InputMousePosWorld();

        FixedList<Entity *, MAX_ENTITIES> & entities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();
        entityPool.GatherActiveObjs( entities );

        FixedList<Entity *, MAX_ENTITIES> & selectedEntities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();

        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) ) {
            selectionStartDragPos = mousePosWorld;
            selectionEndDragPos = mousePosWorld;
            selectionDragging = true;
        }

        if( mouseHasMoved && selectionDragging ) {
            selectionEndDragPos = mousePosWorld;
        }

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities[ entityIndex ];
            switch( ent->type ) {
                case ENTITY_TYPE_INVALID: break;
                case ENTITY_TYPE_SHIP_BEGIN: break;
                case ENTITY_TYPE_SHIP_A:
                {
                    if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) && selectionDragging ) {
                        Circle c = {};
                        c.pos = ent->pos;
                        c.rad = 32.0f;
                        if( selectionEndDragPos - selectionStartDragPos == glm::vec2( 0 ) ) {
                            ent->selected = c.Contains( mousePosWorld );
                        }
                        else {
                            BoxBounds bb = {};
                            bb.min = glm::min( selectionStartDragPos, selectionEndDragPos );
                            bb.max = glm::max( selectionStartDragPos, selectionEndDragPos );
                            ent->selected = bb.Intersects( c );
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

                    ArrivalCircle * arrivalCircle = arrivalCirclePool.Get( ent->dest.arrivalCircle );
                    if( arrivalCircle != nullptr ) {
                        const f32 targetRad = arrivalCircle->targetRad;
                        const f32 slowRad = arrivalCircle->slowRad;
                        const f32 timeToTarget = arrivalCircle->timeToTarget;

                        //core->RenderDrawCircle( arrivalCircle->pos, targetRad, glm::vec4( 0, 1, 0, 0.8f ) );
                        //core->RenderDrawCircle( arrivalCircle->pos, slowRad, glm::vec4( 1, 0, 0, 0.8f ) );

                        glm::vec2 dir = arrivalCircle->pos - ent->pos;
                        f32 dist = glm::length( dir );

                        if( dist < targetRad ) {
                            ent->dest.moving = false;
                        }

                        if( ent->dest.moving == true ) {
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

                        if( ent->dest.moving == false ) {
                            arrivalCircle->targetRad = glm::max( arrivalCircle->targetRad, dist );
                        }
                    }

                    ent->vel += steer * dt;
                    if( ent->dest.moving == false ) {
                        ent->vel *= drag;
                    }

                    if( glm::length2( ent->vel ) > maxSpeed2 ) {
                        ent->vel = glm::normalize( ent->vel );
                        ent->vel = ent->vel * maxSpeed;
                    }

                    //if( glm::length2( ent->vel ) > 0.1f ) {
                    //    ent->ori = glm::atan( ent->vel.x, ent->vel.y );
                    //}

                    ent->pos += ent->vel * dt;

                } break;
                case ENTITY_TYPE_SHIP_END: break;
                case ENTITY_TYPE_ENEMY: break;
                default: break;
            }
        }

        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * entA = entities[ entityIndexA ];
            if( entA->hasCollision ) {
                for( i32 entityIndexB = entityIndexA + 1; entityIndexB < entityCount; entityIndexB++ ) {
                    Entity * entB = entities[ entityIndexB ];
                    if( entB->hasCollision ) {
                        Circle ca = entA->GetCollisionCircleWorld();
                        Circle cb = entB->GetCollisionCircleWorld();

                        Manifold m = {};
                        if( ca.Collision( cb, m ) ) {
                            entA->pos -= m.normal * m.penetration * 0.5f;
                            entB->pos += m.normal * m.penetration * 0.5f;
                        }
                    }
                }
            }
        }

        FixedList<ArrivalCircle *, MAX_ENTITIES> & activeArrivalCircles = *core->MemoryAllocateTransient<FixedList<ArrivalCircle *, MAX_ENTITIES>>();
        arrivalCirclePool.GatherActiveObjs( activeArrivalCircles );
        const i32 activeArrivalCirclesCount = activeArrivalCircles.GetCount();
        for( int arrivalCircleIndex = 0; arrivalCircleIndex < activeArrivalCirclesCount; arrivalCircleIndex++ ) {
            ArrivalCircle * c = activeArrivalCircles[ arrivalCircleIndex ];
            for( int entHandleIndex = 0; entHandleIndex < c->ents.GetCount(); entHandleIndex++ ) {
                EntityHandle h = c->ents[ entHandleIndex ];
                Entity * ent = entityPool.Get( h );
                if( ent == nullptr || ent->dest.arrivalCircle != c->id || ent->dest.moving == false ) {
                    c->ents.RemoveValue( h );
                    entHandleIndex--;
                    continue;
                }
            }

            if( c->ents.GetCount() == 0 ) {
                arrivalCirclePool.Remove( c->id );
            }
        }


        if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) && selectionDragging ) {
            selectionDragging = false;
            selectionEndDragPos = glm::vec2( 0 );
            selectionStartDragPos = glm::vec2( 0 );
        }

        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) ) {
            ArrivalCircleHandle arrivalHandle = {};
            ArrivalCircle * circle = arrivalCirclePool.Add( arrivalHandle );
            Assert( circle != nullptr );

            circle->id = arrivalHandle;
            circle->targetRad = 45.0f;
            circle->slowRad = 128.0f;
            circle->timeToTarget = 0.1f;
            circle->pos = mousePosWorld;
            circle->aliveHandleCount = selectedEntities.GetCount();

            ShipDestination dest = {};
            dest.arrivalCircle = arrivalHandle;
            dest.moving = true;

            const int selectedEntityCount = selectedEntities.GetCount();
            for( int selectedEntityIndex = 0; selectedEntityIndex < selectedEntityCount; ++selectedEntityIndex ) {
                Entity * ent = selectedEntities[ selectedEntityIndex ];
                circle->ents.Add( ent->id );
                ent->dest = dest;
            }
        }
    }

    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)(b->pos.y - a->pos.y);
    }

    void GameModeGame::Render( Core * core, f32 dt ) {
        FixedList<Entity *, MAX_ENTITIES> & entities = *core->MemoryAllocateTransient<FixedList<Entity *, MAX_ENTITIES>>();
        entityPool.GatherActiveObjs( entities );

        entities.Sort( &YSortEntities );

        const int entityCount = entities.GetCount();
        for( int entityIndex = 0; entityIndex < entityCount; ++entityIndex ) {
            Entity * ent = entities[ entityIndex ];
            switch( ent->type ) {
                case ENTITY_TYPE_INVALID: break;
                case ENTITY_TYPE_SHIP_BEGIN: break;
                case ENTITY_TYPE_SHIP_A:
                {
                    if( ent->selected ) {
                        core->RenderDrawSprite( sprSelectionCircle, ent->pos, 0.0f, glm::vec2( 0.5f ) );
                    }

                    core->RenderDrawSprite( spr_RedWorker, ent->pos, ent->ori, glm::vec2( 1 ) );
                    //core->RenderDrawRect( ent->pos, glm::vec2( 32 ), 0.0f );

                } break;
                case ENTITY_TYPE_SHIP_END: break;
                case ENTITY_TYPE_ENEMY: break;
                default: break;
            }
        }

    #if 0
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * entA = entities[ entityIndexA ];
            if( entA->hasCollision ) {
                Circle c = entA->GetCollisionCircleWorld();
                core->RenderDrawCircle( c.pos, c.rad, glm::vec4( 0.5f, 0.5f, 1, 0.8f ) );
            }
        }
    #endif

        if( selectionDragging == true ) {
            glm::vec2 bl = glm::min( selectionStartDragPos, selectionEndDragPos );
            glm::vec2 tr = glm::max( selectionStartDragPos, selectionEndDragPos );
            core->RenderDrawRect( bl, tr, Colors::BOX_SELECTION_COLOR );
        }

        FontHandle f = core->ResourceGetFont( "default" );
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
            entity->dest.moving = false;
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

    Entity * GameModeGame::SpawnEntityShipA( glm::vec2 pos ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_SHIP_A, pos );
        if( ent != nullptr ) {
            ent->hasCollision = true;
            ent->collisionCircle.rad = 20.0f;
        }

        return ent;
    }

}
