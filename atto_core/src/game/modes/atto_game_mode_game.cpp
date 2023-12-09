#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)( b->pos.y - a->pos.y );
    }

    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
        spr_RedWorker = core->ResourceGetAndLoadTexture( "kenny_sprites_03/unit/scifiUnit_11.png" );
        spr_Structure_Hub = core->ResourceGetAndLoadTexture( "kenny_sprites_03/structure/scifiStructure_04.png" );
        sprShipB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/ship_B.png" );
        sprEnemyA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_A.png" );
        sprEnemyB = core->ResourceGetAndLoadTexture( "kenny_sprites_01/enemy_B.png" );
        sprStationA = core->ResourceGetAndLoadTexture( "kenny_sprites_01/station_A.png" );
        sprSelectionCircle = core->ResourceGetAndLoadTexture( "unit_selection_01.png" );

    #if 1
        SpawnEntityUnitWorker( glm::vec2( 400, 400 ) );
        SpawnEntityUnitWorker( glm::vec2( 200, 400 ) );

        SpawnEntityUnitWorker( glm::vec2( 400, 600 ) );
        SpawnEntityUnitWorker( glm::vec2( 200, 600 ) );

        SpawnEntityUnitWorker( glm::vec2( 400, 200 ) );
        SpawnEntityUnitWorker( glm::vec2( 200, 200 ) );
    #else 
        for( int x = 0; x < 20; x++ ) {
            for( int y = 0; y < 20; y++ ) {
                SpawnEntityUnitWorker( glm::vec2( x, y ) * 30.0f + glm::vec2(800, 200));
            }
        }

    #endif
        SpawnEntityStructureHub( glm::vec2( 1000, 1000 ) );
        
    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt ) {
        const bool mouseHasMoved = core->InputMouseHasMoved();
        const glm::vec2 mousePosNDC = core->InputMousePosNDC();
        const glm::vec2 mousePosWorld = core->InputMousePosWorld();
        //core->LogOutput( LogLevel::DEBUG, "MOUSE NDC: %f, %f", mousePosNDC.x, mousePosNDC.y );
        //core->LogOutput( LogLevel::DEBUG, "MOUSE WORLD: %f, %f", mousePosWorld.x, mousePosWorld.y );

        activeEntities.Clear( true );
        entityPool.GatherActiveObjs( activeEntities );

        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) ) {
            selectionStartDragPos = mousePosWorld;
            selectionEndDragPos = mousePosWorld;
            selectionDragging = true;
        }

        if( mouseHasMoved && selectionDragging ) {
            selectionEndDragPos = mousePosWorld;
        }

        selectedEntities.Clear( true );
        const i32 activeEntityCount = activeEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
            Entity * ent = activeEntities[ entityIndex ];
            if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) && selectionDragging ) {
                Collider c = ent->GetSelectionColliderWorld();
                if( selectionEndDragPos - selectionStartDragPos == glm::vec2( 0 ) ) {
                    ent->selected = c.Contains( mousePosWorld );
                }
                else {
                    Collider bb = {};
                    bb.type = COLLIDER_TYPE_BOX;
                    bb.box.min = glm::min( selectionStartDragPos, selectionEndDragPos );
                    bb.box.max = glm::max( selectionStartDragPos, selectionEndDragPos );
                    ent->selected = c.Intersects( bb );
                }
            }

            if( ent->selected ) {
                selectedEntities.Add( ent );
            }
        }

        DrawContext * uiDraws = core->RenderGetDrawContext( 1 );
        bool isMouseOverUI = false;
        {
          //FontHandle f = core->ResourceGetFont( "default" );
          //core->RenderDrawText( f, glm::vec2( 100, 100 ), 128.0f, "Hello World" );
            bool hasBuilding = SelectionHasType( ENTITY_TYPE_STRUCTURE_HUB );
            BoxBounds bb = {};
            bb.min = glm::vec2( -1, -1 );
            bb.max = glm::vec2( 1, -0.777f );
            isMouseOverUI = bb.Contains( mousePosNDC );
            uiDraws->RenderDrawRectNDC( glm::vec2( -1, -1 ), glm::vec2( 1, -0.777f ), Colors::MIDNIGHT_BLUE );
            glm::vec2 start = glm::vec2( -1, -1 );
            glm::vec2 end = glm::vec2( 1, -0.777f );
            i32 count = 10;
            f32 dx = ( end.x - start.x ) / (f32)count;
            f32 padding = 0.025f;
            start.x -= dx;
            end.x = start.x + dx;
            bool once = false;
            for( int i = 0; i < 10; i++ ) {
                start.x += dx;
                end.x = start.x + dx;
                uiDraws->RenderDrawRectNDC( start + padding, end - padding );
                if( hasBuilding && once == false ) {
                    once = true;
                    uiDraws->RenderDrawSpriteNDC( spr_RedWorker, ( ( end - padding ) + ( start + padding ) ) / 2.0f, 0.0f, glm::vec2( 2.0f ) );
                }
            }
        }

        for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
            Entity * ent = activeEntities[ entityIndex ];
            switch( ent->type ) {
                case ENTITY_TYPE_INVALID: break;
                case ENTITY_TYPE_UNITS_BEGIN: break;
                case ENTITY_TYPE_UNIT_WORKER:
                {
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
                case ENTITY_TYPE_UNITS_END: break;
                case ENTITY_TYPE_ENEMY: break;
                default: break;
            }
        }

        for( i32 entityIndexA = 0; entityIndexA < activeEntityCount; entityIndexA++ ) {
            Entity * entA = activeEntities[ entityIndexA ];
            if( entA->hasCollision ) {
                for( i32 entityIndexB = entityIndexA + 1; entityIndexB < activeEntityCount; entityIndexB++ ) {
                    Entity * entB = activeEntities[ entityIndexB ];
                    if( entB->hasCollision ) {
                        Collider ca = entA->GetCollisionCircleWorld();
                        Collider cb = entB->GetCollisionCircleWorld();

                        Manifold m = {};
                        if( ca.Collision( cb, m ) ) {
                            if( entA->isCollisionStatic == true ) {
                                entB->pos += m.normal * m.penetration;
                            }
                            else if( entB->isCollisionStatic == true ) {
                                entA->pos -= m.normal * m.penetration;
                            }
                            else {
                                entA->pos -= m.normal * m.penetration * 0.5f;
                                entB->pos += m.normal * m.penetration * 0.5f;
                            }
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

        f32 cameraSpeed = 200.0f;
        glm::vec2 cameraPos = core->GetCameraPos();
        //core->LogOutput( LogLevel::DEBUG, "CameraPos: %f, %f", cameraPos.x, cameraPos.y );

        if( core->InputKeyDown( KEY_CODE_W ) ) {
            cameraPos.y += cameraSpeed * dt;
        }
        if( core->InputKeyDown( KEY_CODE_S ) ) {
            cameraPos.y -= cameraSpeed * dt;
        }
        if( core->InputKeyDown( KEY_CODE_A ) ) {
            cameraPos.x -= cameraSpeed * dt;
        }
        if( core->InputKeyDown( KEY_CODE_D ) ) {
            cameraPos.x += cameraSpeed * dt;
        }
        core->SetCameraPos( cameraPos );


        activeEntities.Sort( &YSortEntities );
        DrawContext * dctx = core->RenderGetDrawContext( 0 );

        for( int entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
            Entity * ent = activeEntities[ entityIndex ];
            if( ent->sprite != nullptr ) {
                //if( ent->selected ) {
                //    core->RenderDrawSprite( sprSelectionCircle, ent->pos, 0.0f, glm::vec2( 0.5f ) );
                //}
                glm::vec4 color = ent->selected ? glm::vec4( 0.6f, 1.2f, 0.6f, 1.0f ) : glm::vec4( 1 );
                dctx->RenderDrawSprite( ent->sprite, ent->pos, ent->ori, glm::vec2( 1 ), color );
                //core->RenderDrawRect( ent->pos, glm::vec2( 32 ), 0.0f );
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
    #if 0
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * ent = entities[ entityIndexA ];
            if( ent->isSelectable ) {
                Collider c = ent->GetSelectionColliderWorld();
                switch( c.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        core->RenderDrawCircle( c.circle.pos, c.circle.rad, glm::vec4( 0.5f, 0.5f, 1, 0.8f ) );
                    } break;
                    case COLLIDER_TYPE_BOX:
                    {
                        core->RenderDrawRect( c.box.min, c.box.max, glm::vec4( 0.5f, 0.5f, 1, 0.8f ) );
                    } break;
                }
            }
        }
    #endif
        if( selectionDragging == true ) {
            glm::vec2 bl = glm::min( selectionStartDragPos, selectionEndDragPos );
            glm::vec2 tr = glm::max( selectionStartDragPos, selectionEndDragPos );
            dctx->RenderDrawRect( bl, tr, Colors::BOX_SELECTION_COLOR );
        }

        core->RenderSubmit( dctx, true );
        core->RenderSubmit( uiDraws, false );
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

    Entity * GameModeGame::SpawnEntityUnitWorker( glm::vec2 pos ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_UNIT_WORKER, pos );
        if( ent != nullptr ) {
            ent->sprite = spr_RedWorker;
            ent->hasCollision = true;
            ent->isSelectable = true;
            ent->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
            ent->selectionCollider.circle.rad = 22.0f;

            ent->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
            ent->collisionCollider.circle.rad = 20.0f;
        }

        return ent;
    }


    Entity * GameModeGame::SpawnEntityStructureHub( glm::vec2 pos ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_STRUCTURE_HUB, pos );
        if( ent != nullptr ) {
            ent->sprite = spr_Structure_Hub;
            ent->isSelectable = true;
            ent->selectionCollider.type = COLLIDER_TYPE_BOX;
            ent->selectionCollider.box.min = glm::vec2( -64 );
            ent->selectionCollider.box.max = glm::vec2( 64 );

            ent->hasCollision = true;
            ent->isCollisionStatic = true;
            ent->collisionCollider.type = COLLIDER_TYPE_BOX;
            ent->collisionCollider.box.min = glm::vec2( -64 );
            ent->collisionCollider.box.max = glm::vec2( 64 );
        }

        return ent;
    }

    bool GameModeGame::SelectionHasType( EntityType type ) {
        const i32 entsCount = selectedEntities.GetCount();
        for( i32 i = 0; i < entsCount; i++ ) {
            Entity * ent = selectedEntities[ i ];
            if( ent->type == type ) {
                return true;
            }
        }
        return false;
    }

    Collider Entity::GetSelectionColliderWorld() const {
        Collider c = selectionCollider;
        c.Translate( pos );
        return c;
    }


    Collider Entity::GetCollisionCircleWorld() const {
        Collider c = collisionCollider;
        c.Translate( pos );
        return c;
    }

}
