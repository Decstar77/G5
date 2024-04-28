#include "atto_sim_map.h"
#include "../shared/atto_logging.h"

namespace atto {
    void SimMap::SimInitialize() {
        // @TODO: This won't work if we have more than one map instance !!
        if ( GlobalRpcTable[ 1 ] == nullptr ) {
            GlobalRpcTable[ (i32)MapAction::REQUEST_SELECTION_ENTITIES ] = new RpcMemberFunction( this, &SimMap::Action_RequestSelectEntities );
            GlobalRpcTable[ (i32)MapAction::REQUEST_MOVE ] = new RpcMemberFunction( this, &SimMap::Action_RequestMove );
            GlobalRpcTable[ (i32)MapAction::COMMAND_MOVE ] = new RpcMemberFunction( this, &SimMap::Action_CommandMove );
            
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
        simTime += dt;

        // Actions

        // SendToAllClients()
    }

    void SimMap::ApplyActions( MapActionBuffer * actionBuffer ) {
        ZeroStruct( simActionBuffer );

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
        const i32 entityCount = actionActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = actionActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && IsUnitType(ent->type) && ent->selectedBy.Contains( playerNumber ) ) {
                f32 activeTime = simTime;
                const glm::vec2 starting = ent->posTimeline.ValueForTime( activeTime );
                const f32 speed = 100.0f;
                const f32 dist = glm::distance( starting, p );
                const f32 travelTime = dist / speed;
                ent->posTimeline.RemoveKeyFramesPast( activeTime );
                ent->posTimeline.AddKeyFrame( starting, activeTime );
                ent->posTimeline.AddKeyFrame( p, activeTime + travelTime );
                simActionBuffer.Command_MoveUnit( ent->handle, starting, p, activeTime, activeTime + travelTime );
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
            entity->posTimeline.AddKeyFrame( createInfo.pos, simTime );
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

    void SimMap::Action_CommandMove( EntityHandle entityHandle, glm::vec2 p1, glm::vec2 p2, f32 t1, f32 t2 ) {
        SimEntity * ent = entityPool.Get( entityHandle );
        if ( ent != nullptr ) {
            ent->posTimeline.RemoveKeyFramesPast( t1 );
            ent->posTimeline.AddKeyFrame( p1, t1 );
            ent->posTimeline.AddKeyFrame( p2, t2 );
        }
    }

}

