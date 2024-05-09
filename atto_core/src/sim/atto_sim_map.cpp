#include "atto_sim_map.h"
#include "../shared/atto_resources.h"
#include "../shared/atto_logging.h"

namespace atto {
    void SimMap::SimInitialize() {
        // @TODO: This won't work if we have more than one map instance !!
        if ( GlobalRpcTable[ 1 ] == nullptr ) {
            GlobalRpcTable[ (i32)MapAction::REQUEST_SELECTION_ENTITIES ] = new RpcMemberFunction( this, &SimMap::Action_RequestSelectEntities );
            GlobalRpcTable[ (i32)MapAction::REQUEST_MOVE ] = new RpcMemberFunction( this, &SimMap::Action_RequestMove );
            GlobalRpcTable[ (i32)MapAction::REQUEST_ATTACK ] = new RpcMemberFunction( this, &SimMap::Action_RequestAttack );
            GlobalRpcTable[ (i32)MapAction::REQUEST_PLACE_STRUCTURE ] = new RpcMemberFunction( this, &SimMap::Action_RequestPlaceStructure );
        }

        SpriteResourceCreateInfo spriteCreateInfo = {};
        spriteCreateInfo.frameXCount = 8;
        spriteCreateInfo.frameYCount = 8;
        spriteCreateInfo.frameCount = spriteCreateInfo.frameXCount * spriteCreateInfo.frameYCount;
        spriteCreateInfo.frameWidth = 8;
        spriteCreateInfo.frameHeight = 8;
        SpriteResource * sprTile = ResourceGetAndCreateSprite( "res/game/tilesets/tileset_sand.png", spriteCreateInfo );
        tileMap.Fill( sprTile, 0, 0, 0 );
        tileMap.FillBorder( sprTile, 1, 0, TILE_FLAG_UNWALKABLE );

        //for( i32 yIndex = 0; yIndex < TILE_MAX_HEIGHT; yIndex++ ) {
        //    for( i32 xIndex = 0; xIndex < TILE_MAX_WIDTH; xIndex++ ) {
        //        i32 spriteX = Random::Int(0 ,3);
        //        i32 spriteY = Random::Int(0 ,3);
        //        tileMap.PlaceTile( xIndex, yIndex, sprTile, spriteX, spriteY, 0 );
        //    }
        //}

        PlayerNumber p1 = PlayerNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );

        TeamNumber t1 = TeamNumber::Create( 1 );
        TeamNumber t2 = TeamNumber::Create( 2 );

        playerMonies.SetCount( 2 ); 
        playerMonies[ p1.value - 1 ].cash = 100;
        playerMonies[ p1.value - 1 ].power = 50;
        playerMonies[ p2.value - 1 ].cash = 100;
        playerMonies[ p2.value - 1 ].power = 50;

        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 200 ), p1, t1 );
        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 240 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 260 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 280 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 300 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 320 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 340 ), p1, t1 );
        //Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 240, 360 ), p1, t1 );
        Action_CommandSpawnEntity( EntityType::STRUCTURE_CITY_CENTER, tileMap.PosToTileBL( Fp2( 200, 200 ) ) , p1, t1 );
        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, Fp2( 400, 200 ), p2, t2 );
    }

    bool SimMap::SimDoneTicks() {
        return simTickNumber == simTickStopNumber;
    }

    void SimMap::SimNextTick( f32 dt ) {
        simTimeAccum += dt;
        if ( simTimeAccum >= tickTime ) {
            simTimeAccum -= tickTime;
            if ( simTickNumber == simTickStopNumber ) {
                ATTOWARN( "Can't tick anymore :( " );
                return;
            }

            simTickNumber++;

            actionActiveEntities.Clear();
            entityPool.GatherActiveObjs( actionActiveEntities );

            const i32 entityCount = actionActiveEntities.GetCount();
            for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = actionActiveEntities[ entityIndex ];
                ent->lastPos = ent->pos;
                ent->posTime = 0.0f;

                FpCollider entCollider = ent->ColliderFpWorldSpace( ent->pos );

                if ( ent->unit.state == UnitState::MOVING ) {
                    fp2 dir = FpNormalize( ent->dest - ent->pos ) * ent->unit.speed;
                    ent->pos = ent->pos + dir * tickTimeFp;

                    if ( FpDistance( ent->pos, ent->dest ) < Fp( 5 ) ) {
                        ent->unit.state = UnitState::IDLE;
                    }
                } else if ( ent->unit.state == UnitState::ATTACKING ) {
                    SimEntity * targetEnt = entityPool.Get( ent->target );
                    if ( targetEnt != nullptr ) {
                        if ( FpDistance( ent->pos, targetEnt->pos ) > ent->unit.range ) {
                            fp2 dir = FpNormalize( targetEnt->pos - ent->pos ) * ent->unit.speed;
                            ent->pos = ent->pos + dir * tickTimeFp;
                        } else {

                        }
                    }
                }
            }

            for ( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                SimEntity * entA = actionActiveEntities[ entityIndexA ];
                if ( entA->movable == false ) { continue; }
                FpCollider entACollider = entA->ColliderFpWorldSpace( entA->pos );
                for ( i32 entityIndexB = 0; entityIndexB < entityCount; entityIndexB++ ) {
                    if ( entityIndexA == entityIndexB ) {
                        continue;
                    }

                    SimEntity * entB = actionActiveEntities[ entityIndexB ];
                    FpCollider entBCollider = entB->ColliderFpWorldSpace( entB->pos );

                    FpManifold man = {};
                    if ( FpColliderCollision( entACollider, entBCollider, man ) == true ) {
                        if ( entB->movable == true ) {
                            entA->pos = entA->pos - man.normal * man.penetration / Fp( 2 );
                            entB->pos = entB->pos + man.normal * man.penetration / Fp( 2 );
                        } else { 
                            entA->pos = entA->pos - man.normal * man.penetration;
                        }
                    }
                }
            }
        }
    }

    void SimMap::SimNextTurn( MapTurn * player1Turn, MapTurn * player2Turn, i32 tickCount ) {
        simTickStopNumber += tickCount;
        if ( player1Turn != nullptr ) {
            ApplyActions( &player1Turn->actions );
        }
        if ( player2Turn != nullptr ) {
            ApplyActions( &player2Turn->actions );
        }
    }

    bool SimMap::SimCanPlaceStructure( EntityType type, fp2 pos ) {
        utilActiveEntities.Clear();
        entityPool.GatherActiveObjs( utilActiveEntities );

        Assert( type == EntityType::STRUCTURE_CITY_CENTER || type == EntityType::STRUCTURE_SMOL_REACTOR );

        FpCollider collider = {};
        collider.type = ColliderType::COLLIDER_TYPE_AXIS_BOX;
        if ( type == EntityType::STRUCTURE_CITY_CENTER ) {
            collider.box.min = Fp2( -12, -12 );
            collider.box.max = Fp2( 12, 12 );
        } else if ( type == EntityType::STRUCTURE_SMOL_REACTOR ) {
            collider.box.min = Fp2( -8, -8 );
            collider.box.max = Fp2( 8, 8 );
        }

        TileInterval inv = tileMap.IntervalForCollider( collider, pos );
        bool unwalkable = tileMap.ContainsFlag( inv.xIndex, inv.yIndex, inv.xCount, inv.yCount, TILE_FLAG_UNWALKABLE );
        if ( unwalkable == true ) {
            return false;
        } else {
            return true;
        }

        //i32 entityCount = utilActiveEntities.GetCount();
        //for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
        //    SimEntity * ent = utilActiveEntities[ entityIndex ];
        //    if ( IsStructureType( ent->type ) == true ) {
        //        //ent->collider
        //    }
        //}
    }

    void SimMap::ApplyActions( MapActionBuffer * actionBuffer ) {
        // @NOTE: For results from these rpc actions
        actionActiveEntities.Clear();
        entityPool.GatherActiveObjs( actionActiveEntities );

        // @TODO: This could be pulled out into the rpc system.
        char * turnData = actionBuffer->data.GetBuffer();
        const i32 turnSize = actionBuffer->data.GetSize();

        i32 offset = 0;
        while( offset < turnSize ) {
            MapAction actionType = (MapAction)turnData[ offset ];
            if( actionType == MapAction::NONE ) {
                ATTOERROR( "Can't apply a none turn" );
                break;
            }

            offset += sizeof( MapAction );

            RpcHolder * holder = GlobalRpcTable[ (i32)actionType ];
            holder->Call( turnData + offset );
            i32 lastCallSize = holder->GetLastCallSize();
            offset += lastCallSize;
        }
    }

    void SimMap::Action_RequestMove( PlayerNumber playerNumber, fp2 p ) {
        ATTOTRACE( "PlayerNumber %d requesting move %f, %f", playerNumber.value, ToFloat( p.x ), ToFloat ( p.y ) );

        const i32 entityCount = actionActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = actionActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && IsUnitType( ent->type ) && ent->selectedBy.Contains( playerNumber ) ) {
                ent->dest = p;
                ent->target = EntityHandle::INVALID;
                ent->unit.state = UnitState::MOVING;
            }
        }
    }

    void SimMap::Action_RequestAttack( PlayerNumber playerNumber, EntityHandle handle ) {
        ATTOTRACE( "PlayerNumber %d requesting attack", playerNumber.value );

        SimEntity * targetEnt = entityPool.Get( handle );
        const i32 entityCount = actionActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = actionActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && IsUnitType( ent->type ) && ent->selectedBy.Contains( playerNumber ) ) {
                fp2 dir = FpNormalize( ent->pos - targetEnt->pos ); // @TODO: Check for Nan/zero vec
                ent->dest = targetEnt->pos + dir * Fp( 25 );
                ent->target = targetEnt->handle;
                ent->unit.state = UnitState::ATTACKING;
            }
        }
    }

    void SimMap::Action_RequestPlaceStructure( PlayerNumber playerNumber, i32 entityType, fp2 p ) {
        EntityType type = ( EntityType )entityType;
        ATTOTRACE( "PlayerNumber %d requesting place structure %d, at %f ,%f", playerNumber.value, type, ToFloat( p.x ), ToFloat( p.y ) );
        Action_CommandSpawnEntity( type, p, playerNumber, TeamNumber::Create( 1 ) );
    }

    void SimMap::Action_RequestSelectEntities( PlayerNumber playerNumber, Span<EntityHandle> entities ) {
        // @SAFTY: We should check the player is who they say they are ? Against the peer ids probably
        ATTOTRACE( "PlayerNumber %d selecting %d entities", playerNumber.value, entities.GetCount() );

        i32 entityCount = actionActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = actionActiveEntities[ entityIndex ];
            ent->selectedBy.RemoveValue( playerNumber );
        }

        entityCount = entities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            EntityHandle entityHandle = entities[ entityIndex ];
            SimEntity * ent = entityPool.Get( entityHandle );
            if ( ent != nullptr ) {
                ent->selectedBy.AddUnique( playerNumber );
            }
        }
    }

    SimEntity * SimMap::Action_CommandSpawnEntity( EntitySpawnCreateInfo createInfo ) {
        EntityHandle handle = {};
        SimEntity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if ( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->handle = handle;
            entity->type = createInfo.type;
            entity->playerNumber = createInfo.playerNumber;
            entity->teamNumber = createInfo.teamNumber;
            entity->pos = createInfo.pos;
            entity->lastPos = entity->pos;
            entity->vis = VisMap_OnSpawnEntity( createInfo );
            entity->movable = false;

            if ( entity->type == EntityType::UNIT_SCOUT ) {
                entity->movable = true;
                entity->collider.type = ColliderType::COLLIDER_TYPE_AXIS_BOX;
                entity->collider.box.min = Fp2( -6, -6 );
                entity->collider.box.max = Fp2( 6, 6 );
                entity->unit.range = Fp( 25 );
                entity->unit.speed = Fp( 100 );
            } else if ( entity->type == EntityType::STRUCTURE_CITY_CENTER ) {
                entity->collider.type = ColliderType::COLLIDER_TYPE_AXIS_BOX;
                entity->collider.box.min = Fp2( -12, -12 );
                entity->collider.box.max = Fp2( 12, 12 );

                TileInterval interval = tileMap.IntervalForCollider( entity->collider, entity->pos );
                tileMap.MarkTilesBL( interval.xIndex, interval.yIndex, interval.xCount, interval.yCount, TILE_FLAG_UNWALKABLE );
            } else if ( entity->type == EntityType::STRUCTURE_SMOL_REACTOR ) {
                entity->collider.type = ColliderType::COLLIDER_TYPE_AXIS_BOX;
                entity->collider.box.min = Fp2( -8, -8 );
                entity->collider.box.max = Fp2( 8, 8 );

                TileInterval interval = tileMap.IntervalForCollider( entity->collider, entity->pos );
                tileMap.MarkTilesBL( interval.xIndex, interval.yIndex, interval.xCount, interval.yCount, TILE_FLAG_UNWALKABLE );
            }
        }

        return entity;
    }

    SimEntity * SimMap::Action_CommandSpawnEntity( EntityType type, fp2 pos, PlayerNumber playerNumber, TeamNumber teamNumber ) {
        EntitySpawnCreateInfo createInfo = {};
        createInfo.pos = pos;
        createInfo.playerNumber = playerNumber;
        createInfo.teamNumber = teamNumber;
        createInfo.type = type;
        return Action_CommandSpawnEntity( createInfo );
    }

}

