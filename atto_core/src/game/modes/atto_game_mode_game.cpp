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
        spr_BlueWorker_Idle = core->ResourceGetAndLoadTexture( "unit_basic_man.png" );
        spr_BlueWorker_Firing = core->ResourceGetAndLoadTexture( "unit_basic_man_firing.png" );
        spr_BlueWorker_Firing_Muzzel = core->ResourceGetAndLoadTexture( "unit_basic_man_firing_muzzel.png" );
        spr_BlueWorker_Selection = core->ResourceGetAndLoadTexture( "unit_basic_man_selection.png" );
        spr_RedWorker = core->ResourceGetAndLoadTexture( "unit_basic_enemy.png" );
        spr_Structure_Hub = core->ResourceGetAndLoadTexture( "unit_basic_man.png" );

    #if 1
        SpawnEntityUnitWorker( glm::vec2( 40, 40 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 20, 40 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 40, 60 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 20, 60 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 40, 20 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 20, 20 ), 1 );
        SpawnEntityUnitWorker( glm::vec2( 80, 20 ), 2 );
        SpawnEntityUnitWorker( glm::vec2( 60, 20 ), 2 );
    #else 
        for( int x = 0; x < 20; x++ ) {
            for( int y = 0; y < 20; y++ ) {
                SpawnEntityUnitWorker( glm::vec2( x, y ) * 30.0f + glm::vec2( 800, 200 ) );
            }
        }

    #endif
        //SpawnEntityStructureHub( glm::vec2( 100, 100 ), 1 );

    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt ) {
        const bool mouseHasMoved = core->InputMouseHasMoved();
        const glm::vec2 mousePosNDC = core->InputMousePosNDC();
        const glm::vec2 mousePosWorld = core->InputMousePosWorld();
        Entity * hoveredEnt = nullptr;
        //core->LogOutput( LogLevel::DEBUG, "MOUSE NDC: %f, %f", mousePosNDC.x, mousePosNDC.y );
        //core->LogOutput( LogLevel::DEBUG, "MOUSE WORLD: %f, %f", mousePosWorld.x, mousePosWorld.y );

        // @NOTE: Gather active entities
        activeEntities.Clear( true );
        entityPool.GatherActiveObjs( activeEntities );
        const i32 activeEntityCount = activeEntities.GetCount();

        // @NOTE: Gather selected entities and find hovered entity
        selectedEntities.Clear( true );
        for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
            Entity * ent = activeEntities[ entityIndex ];
            if( ent->selected ) {
                selectedEntities.Add( ent );
            }

            Collider c = ent->GetSelectionColliderWorld();
            if( hoveredEnt == nullptr && c.Contains( mousePosWorld ) ) {
                hoveredEnt = ent;
            }
        }

        DrawContext * spriteDraws = core->RenderGetDrawContext( 0 );
        DrawContext * uiDraws = core->RenderGetDrawContext( 1 );
        DrawContext * debugDraws = core->RenderGetDrawContext( 2 );


        // @NOTE: Draw UI Code
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
                uiDraws->RenderDrawRectNDC( start + padding, end - padding, glm::vec4( 0.7f, 0.7f, 0.7f, 1.0f ) );
                if( hasBuilding && once == false ) {
                    once = true;
                    BoxBounds bb2 = {};
                    bb2.min = start + padding;
                    bb2.max = end - padding;

                    if( bb2.Contains( mousePosNDC ) ) {
                        if( core->InputMouseButtonDown( MOUSE_BUTTON_1 ) ) {
                            uiDraws->RenderDrawRectNDC( start + padding, end - padding, glm::vec4( 0.9f, 0.9f, 0.9f, 1.0f ) );
                        }
                        else {
                            uiDraws->RenderDrawRectNDC( start + padding, end - padding, glm::vec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
                        }

                        if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) ) {
                            for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
                                Entity * ent = activeEntities[ entityIndex ];
                                if( ent->selected && ent->type == ENTITY_TYPE_STRUCTURE_HUB ) {
                                    glm::vec2 spawnPos = ent->pos + glm::vec2( Random::Float( -10, 10 ), 100 );
                                    SpawnEntityUnitWorker( spawnPos, 1 );
                                }
                            }
                        }
                    }

                    uiDraws->RenderDrawSpriteNDC( spr_BlueWorker_Idle, ( ( end - padding ) + ( start + padding ) ) / 2.0f, 0.0f, glm::vec2( 2.0f ) );
                }
            }
        }

        // @NOTE: Update selection if input is for the world
        if( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) ) {
            selectionStartDragPos = mousePosWorld;
            selectionEndDragPos = mousePosWorld;
            selectionDragging = true;
        }

        if( mouseHasMoved && selectionDragging ) {
            selectionEndDragPos = mousePosWorld;
        }

        if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) && selectionDragging ) {
            for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
                Entity * ent = activeEntities[ entityIndex ];

                if( ent->isSelectable == false ) {
                    continue;
                }

                if( ent->teamNumber != playerTeamNumber ) {
                    continue;
                }

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

                if( ent->selected ) {
                    selectedEntities.Add( ent );
                }
            }
        }

        
        const f32 fixedDt = 0.01f;
        const i32 maxIterations = 3;
        dtAccumulator += dt;
        i32 iterations = 0;
        while( dtAccumulator > fixedDt && iterations < maxIterations ) {
            iterations++;
            dtAccumulator -= fixedDt;

            for( i32 entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
                Entity * ent = activeEntities[ entityIndex ];
                switch( ent->type ) {
                    case ENTITY_TYPE_INVALID: break;
                    case ENTITY_TYPE_UNITS_BEGIN: break;
                    case ENTITY_TYPE_UNIT_WORKER:
                    {
                        const f32 acc = 100;
                        const f32 maxSpeed = 50;
                        const f32 maxSpeed2 = maxSpeed * maxSpeed;
                        const f32 drag = 0.90f;
                        glm::vec2 steer = glm::vec2( 0, 0 );

                        Entity * targetEnt = entityPool.Get( ent->target.entity );

                        const f32 myRange = 25.0f;
                        bool inRangeOfTarget = false;
                        if( targetEnt != nullptr && targetEnt->teamNumber != ent->teamNumber ) {
                            //debugDraws->RenderDrawCircle( targetEnt->pos, myRange, glm::vec4( 0, 1, 0, 0.8f ) );

                            f32 targetDist = glm::distance( targetEnt->pos, ent->pos ); // @SPEED(): Use dist 2
                            if( targetDist <= myRange ) {
                                inRangeOfTarget = true;
                            }
                            else {
                                ent->target.moving = true;
                            }
                        }

                        if( inRangeOfTarget == true ) {
                            ent->target.moving = false;
                        }

                        ArrivalCircle * arrivalCircle = arrivalCirclePool.Get( ent->target.arrivalCircle );
                        if( arrivalCircle != nullptr && inRangeOfTarget == false ) {
                            const f32 targetRad = arrivalCircle->targetRad;
                            const f32 slowRad = arrivalCircle->slowRad;
                            const f32 timeToTarget = arrivalCircle->timeToTarget;

                            //core->RenderDrawCircle( arrivalCircle->pos, targetRad, glm::vec4( 0, 1, 0, 0.8f ) );
                            //core->RenderDrawCircle( arrivalCircle->pos, slowRad, glm::vec4( 1, 0, 0, 0.8f ) );

                            glm::vec2 dir = arrivalCircle->pos - ent->pos;
                            f32 dist = glm::length( dir );

                            if( dist < targetRad ) {
                                ent->target.moving = false;
                            }

                            if( ent->target.moving == true ) {
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

                            if( ent->target.moving == false ) {
                                arrivalCircle->targetRad = glm::max( arrivalCircle->targetRad, dist );
                            }
                        }

                        ent->vel += steer * fixedDt;
                        if( ent->target.moving == false ) {
                            ent->vel *= drag;
                        }

                        if( glm::length2( ent->vel ) > maxSpeed2 ) {
                            ent->vel = glm::normalize( ent->vel );
                            ent->vel = ent->vel * maxSpeed;
                        }

                        //if( glm::length2( ent->vel ) > 0.1f ) {
                        //    ent->ori = glm::atan( ent->vel.x, ent->vel.y );
                        //}

                        ent->pos += ent->vel * fixedDt;

                        if( inRangeOfTarget == true ) {
                            ent->spriteCurrent = ent->spriteFiring;
                            ent->fireRateAccumulator += fixedDt;
                            if( ent->fireRateAccumulator >= ent->fireRate ) {
                                ent->fireRateAccumulator = 0.0f;
                                spriteDraws->RenderDrawSprite( spr_BlueWorker_Firing_Muzzel, ent->pos + glm::vec2( 9, -0.5f ) );
                                
                                targetEnt->currentHealth -= ent->fireDamage;
                                if( targetEnt->currentHealth <= 0 ) {
                                    entityPool.Remove( targetEnt->id );
                                }
                            }
                        }
                        else {
                            ent->spriteCurrent = ent->spriteIdle;
                        }

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
        }

        //core->LogOutput( LogLevel::INFO, "Iterations %d", iterations );

        FixedList<ArrivalCircle *, MAX_ENTITIES> & activeArrivalCircles = *core->MemoryAllocateTransient<FixedList<ArrivalCircle *, MAX_ENTITIES>>();
        arrivalCirclePool.GatherActiveObjs( activeArrivalCircles );
        const i32 activeArrivalCirclesCount = activeArrivalCircles.GetCount();
        for( int arrivalCircleIndex = 0; arrivalCircleIndex < activeArrivalCirclesCount; arrivalCircleIndex++ ) {
            ArrivalCircle * c = activeArrivalCircles[ arrivalCircleIndex ];
            for( int entHandleIndex = 0; entHandleIndex < c->ents.GetCount(); entHandleIndex++ ) {
                EntityHandle h = c->ents[ entHandleIndex ];
                Entity * ent = entityPool.Get( h );
                if( ent == nullptr || ent->target.arrivalCircle != c->id || ent->target.moving == false ) {
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
        if( isMouseOverUI == false ) {
            if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) ) {
                ArrivalCircleHandle arrivalHandle = {};
                ArrivalCircle * circle = arrivalCirclePool.Add( arrivalHandle );
                Assert( circle != nullptr );

                circle->id = arrivalHandle;
                circle->targetRad = 4.5f;
                circle->slowRad = 12.8f;
                circle->timeToTarget = 0.1f;
                circle->pos = mousePosWorld;
                circle->aliveHandleCount = selectedEntities.GetCount();

                EntityTarget target = {};
                target.arrivalCircle = arrivalHandle;
                target.moving = true;
                target.entity = hoveredEnt == nullptr ? EntityHandle::INVALID : hoveredEnt->id;

                const int selectedEntityCount = selectedEntities.GetCount();
                for( int selectedEntityIndex = 0; selectedEntityIndex < selectedEntityCount; ++selectedEntityIndex ) {
                    Entity * ent = selectedEntities[ selectedEntityIndex ];
                    circle->ents.Add( ent->id );
                    ent->target = target;
                }
            }
        }

        f32 cameraSpeed = 20.0f;
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

        for( int entityIndex = 0; entityIndex < activeEntityCount; ++entityIndex ) {
            Entity * ent = activeEntities[ entityIndex ];
            if( ent->spriteCurrent != nullptr ) {
                //if( ent->selected ) {
                //    core->RenderDrawSprite( sprSelectionCircle, ent->pos, 0.0f, glm::vec2( 0.5f ) );
                //}
                //glm::vec4 color = ent->selected ? glm::vec4( 0.6f, 1.2f, 0.6f, 1.0f ) : glm::vec4( 1 );
                if( ent->selected ) {
                    spriteDraws->RenderDrawSprite( spr_BlueWorker_Selection, ent->pos - glm::vec2( 0.5f, 8 ) );
                }
                spriteDraws->RenderDrawSprite( ent->spriteCurrent, ent->pos, ent->ori, glm::vec2( 1 ) );
                //core->RenderDrawRect( ent->pos, glm::vec2( 32 ), 0.0f );
            }
        }

    #if 0
        for( i32 entityIndexA = 0; entityIndexA < activeEntityCount; entityIndexA++ ) {
            Entity * entA = activeEntities[ entityIndexA ];
            if( entA->hasCollision ) {
                Collider c = entA->GetSelectionColliderWorld();
                switch( c.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        debugDraws->RenderDrawCircle( c.circle.pos, c.circle.rad, glm::vec4( 0.5f, 0.5f, 1, 0.8f ) );
                    } break;
                    case COLLIDER_TYPE_BOX:
                    {
                        debugDraws->RenderDrawRect( c.box.min, c.box.max, glm::vec4( 0.5f, 0.5f, 1, 0.8f ) );
                    } break;
                }
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
            spriteDraws->RenderDrawRect( bl, tr, Colors::BOX_SELECTION_COLOR );
        }

        core->RenderSubmit( spriteDraws, true );
        core->RenderSubmit( debugDraws, false );
        core->RenderSubmit( uiDraws, false );
    }


    void GameModeGame::Shutdown( Core * core ) {
    }

    Entity * GameModeGame::SpawnEntity( EntityType type, i32 teamNumber ) {
        EntityHandle hdl = {};
        Entity * entity = entityPool.Add( hdl );
        ZeroStructPtr( entity );
        if( entity != nullptr ) {
            entity->id = hdl;
            entity->type = type;
            entity->selected = false;
            entity->target.moving = false;
            entity->teamNumber = teamNumber;
            return entity;
        }

        Assert( false );

        return nullptr;
    }

    Entity * GameModeGame::SpawnEntity( EntityType type, glm::vec2 pos, i32 teamNumber ) {
        Entity * ent = SpawnEntity( type, teamNumber );
        if( ent != nullptr ) {
            ent->pos = pos;
        }

        return ent;
    }

    Entity * GameModeGame::SpawnEntityUnitWorker( glm::vec2 pos, i32 teamNumber ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_UNIT_WORKER, pos, teamNumber );
        if( ent != nullptr ) {
            if( teamNumber == 1 ) {
                ent->spriteCurrent = spr_BlueWorker_Idle;
                ent->spriteIdle = spr_BlueWorker_Idle;
                ent->spriteFiring = spr_BlueWorker_Firing;
            }
            else if( teamNumber == 2 ) {
                ent->spriteCurrent = spr_RedWorker;
                ent->spriteIdle = spr_RedWorker;
                ent->spriteFiring = spr_BlueWorker_Firing;
            }
            else {
                INVALID_CODE_PATH;
            }

            ent->hasCollision = true;
            ent->isSelectable = true;
            ent->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
            ent->selectionCollider.circle.rad = 5;
            ent->selectionCollider.circle.pos.x = -0.5f;
            ent->selectionCollider.circle.pos.y = -2;

            ent->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
            ent->collisionCollider.circle = ent->selectionCollider.circle;

            ent->fireRate = 1.0f;
            ent->fireDamage = 10;
            ent->maxHealth = 50;
            ent->currentHealth = ent->maxHealth;
        }

        return ent;
    }


    Entity * GameModeGame::SpawnEntityStructureHub( glm::vec2 pos, i32 teamNumber ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_STRUCTURE_HUB, pos, teamNumber );
        if( ent != nullptr ) {
            ent->spriteCurrent = spr_Structure_Hub;
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
