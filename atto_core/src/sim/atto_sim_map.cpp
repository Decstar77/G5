#include "atto_sim_map.h"
#include "../shared/atto_logging.h"

namespace atto {
    void SimMap::SimInitialize() {
        // @TODO: This won't work if we have more than one map instance !!
        if ( GlobalRpcTable[ 1 ] == nullptr ) {
            GlobalRpcTable[ (i32)MapAction::REQUEST_SELECTION_ENTITIES ] = new RpcMemberFunction( this, &SimMap::Action_RequestSelectEntities );
            GlobalRpcTable[ (i32)MapAction::REQUEST_MOVE ] = new RpcMemberFunction( this, &SimMap::Action_RequestMove );
            GlobalRpcTable[ (i32)MapAction::REQUEST_ATTACK ] = new RpcMemberFunction( this, &SimMap::Action_RequestAttack );
        }

        PlayerNumber p1 = PlayerNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );

        TeamNumber t1 = TeamNumber::Create( 1 );
        TeamNumber t2 = TeamNumber::Create( 2 );

        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, glm::vec2( 240, 200 ), p1, t1 );
        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, glm::vec2( 240, 240 ), p1, t1 );
        Action_CommandSpawnEntity( EntityType::STRUCTURE_CITY_CENTER, glm::vec2( 200, 200 ), p1, t1 );
        Action_CommandSpawnEntity( EntityType::UNIT_SCOUT, glm::vec2( 400, 200 ), p2, t2 );
    }

    void SimMap::SimUpdate( f32 dt ) {
        ZeroStruct( simActionBuffer );
        streamData.Clear();

        simTime += dt;
        simTimeAccum += dt;
        if ( simTimeAccum >= tickTime ) {
            simTimeAccum -= tickTime;
            streamDataCounter++;

            actionActiveEntities.Clear();
            entityPool.GatherActiveObjs( actionActiveEntities );

            const i32 entityCount = actionActiveEntities.GetCount();
            for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = actionActiveEntities[ entityIndex ];
                ent->lastPos = ent->pos;

                Collider2D entCollider = ent->ColliderWorldSpace( ent->pos );

                if ( ent->unit.state == UnitState::MOVING ) {
                    glm::vec2 dir = glm::normalize( ent->dest - ent->pos ) * 100.0f;
                    ent->pos += dir * tickTime;
                    ent->posTimeline.AddFrame( ent->pos );

                    if ( glm::distance( ent->pos, ent->dest ) < 5.0f ){
                        ent->unit.state = UnitState::IDLE;
                    }

                } else if ( ent->unit.state == UnitState::ATTACKING ) {
                    SimEntity * targetEnt = entityPool.Get( ent->target );
                    if ( targetEnt != nullptr ) {
                        if ( glm::distance( ent->pos, targetEnt->pos ) > 25.0f ){
                            glm::vec2 dir = glm::normalize( targetEnt->pos - ent->pos ) * 100.0f;
                            ent->pos += dir * tickTime;
                        } else {

                        }
                    }
                }
            }

            for ( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                SimEntity * entA = actionActiveEntities[ entityIndexA ];
                Collider2D entACollider = entA->ColliderWorldSpace( entA->pos );
                for ( i32 entityIndexB = 0; entityIndexB < entityCount; entityIndexB++ ) {
                    if ( entityIndexA == entityIndexB ) {
                        continue;
                    }

                    SimEntity * otherEnt = actionActiveEntities[ entityIndexB ];
                    Collider2D entBCollider = otherEnt->ColliderWorldSpace( otherEnt->pos );

                    Manifold2D man = {};
                    if ( entACollider.Collision( entBCollider, man ) == true ) {
                        entA->pos -= man.normal * man.penetration;
                    }
                }
            }

            for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = actionActiveEntities[ entityIndex ];
                if ( ent->lastPos != ent->pos ) {
                    SimStreamData data = {};
                    data.handle = ent->handle;
                    data.pos = ent->pos;
                    ent->posTimeline.AddFrame( ent->pos );
                    streamData.Add ( data );
                }
            }
        }
    }

    void SimMap::ApplyActions( MapActionBuffer * actionBuffer ) {
        ZeroStruct( simActionBuffer );

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

    void SimMap::Action_RequestMove( PlayerNumber playerNumber, glm::vec2 p ) {
        ATTOTRACE( "PlayerNumber %d requesting move %f, %f", playerNumber.value, p.x, p.y );

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
                glm::vec2 dir = glm::normalize( ent->pos - targetEnt->pos ); // @TODO: Check for Nan/zero vec
                ent->dest = targetEnt->pos + dir * 25.0f;
                ent->target = targetEnt->handle;
                ent->unit.state = UnitState::ATTACKING;
            }
        }
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
            entity->posTimeline.AddFrame( entity->pos );
            entity->vis = VisMap_OnSpawnEntity( createInfo );
            entity->collider.type = ColliderType::COLLIDER_TYPE_AXIS_BOX;
            entity->collider.box.min = glm::vec2( -6, -6 );
            entity->collider.box.max = glm::vec2( 6, 6 );
        }

        return entity;
    }

    SimEntity * SimMap::Action_CommandSpawnEntity( EntityType type, glm::vec2 pos, PlayerNumber playerNumber, TeamNumber teamNumber ) {
        EntitySpawnCreateInfo createInfo = {};
        createInfo.pos = pos;
        createInfo.playerNumber = playerNumber;
        createInfo.teamNumber = teamNumber;
        createInfo.type = type;
        return Action_CommandSpawnEntity( createInfo );
    }

}

