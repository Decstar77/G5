#include "atto_sim_map.h"
#include "../../shared/atto_core.h"
#include "../atto_map_communicator.h"

namespace atto {
    void SimMap::Initialize( MapCommunicator * communicator, bool runSim ) {
        this->communicator = communicator;
        this->runSim = runSim;

        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), 1, 1, glm::vec2( 100.0f ), glm::vec2( 0.0f ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), 2, 2, glm::vec2( 500.0f ), glm::vec2( 0.0f ) );
    }

    void SimMap::Update( Core * core, f32 dt ) {
        if( core->NetworkIsConnected() == true ) {
            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
            while( core->NetworkRecieve( msg ) ) {
                switch( msg.type ) {
                    case NetworkMessageType::ENTITY_STATE_UPDATE:
                    {
                        i32 offset = 0;
                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                        //glm::vec2 pos = NetworkMessagePop<glm::vec2>( msg, offset );
                        u16 px = NetworkMessagePop<u16>( msg, offset );
                        u16 py = NetworkMessagePop<u16>( msg, offset );
                        f32 ori = NetworkMessagePop<f32>( msg, offset );
                        
                        SimEntity * ent = entityPool.Get( handle );
                        if( ent != nullptr ) {
                            //ent->pos = pos;
                            ent->pos.x = ( (f32)px / U16_MAX ) * MAX_MAP_SIZE;
                            ent->pos.y = ( (f32)py / U16_MAX ) * MAX_MAP_SIZE;
                            ent->ori = ori;
                        }
                    } break;
                    case NetworkMessageType::ENTITY_REQUEST_MOVE:
                    {
                        i32 offset = 0;
                        i32 playerNumber = NetworkMessagePop<i32>( msg, offset );
                        glm::vec2 pos = NetworkMessagePop<glm::vec2>( msg, offset );
                        Action_Move( playerNumber, pos );
                    } break;
                    case NetworkMessageType::ENTITY_REQUEST_SELECTION:
                    {
                        i32 offset = 0;
                        i32 playerNumber = NetworkMessagePop<i32>( msg, offset );
                        i32 count = NetworkMessagePop<i32>( msg, offset );
                        EntHandleList & selection = *core->MemoryAllocateTransient<EntHandleList>();
                        for( i32 i = 0; i < count; i++ ) {
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            selection.Add( handle );
                        }
                        ENTITY_SELECTION_CHANGE change = NetworkMessagePop<ENTITY_SELECTION_CHANGE>( msg, offset );
                        Action_Select( playerNumber, selection, change );

                        msg.type = NetworkMessageType::ENTITY_UPDATE_SELECTION;
                        core->NetworkSend( msg );
                    } break;
                    case NetworkMessageType::ENTITY_UPDATE_SELECTION:
                    {
                        i32 offset = 0;
                        i32 playerNumber = NetworkMessagePop<i32>( msg, offset );
                        i32 count = NetworkMessagePop<i32>( msg, offset );
                        EntHandleList & selection = *core->MemoryAllocateTransient<EntHandleList>();
                        for( i32 i = 0; i < count; i++ ) {
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            selection.Add( handle );
                        }
                        ENTITY_SELECTION_CHANGE change = NetworkMessagePop<ENTITY_SELECTION_CHANGE>( msg, offset );

                        Action_Select( playerNumber, selection, change );
                    } break;
                    case NetworkMessageType::ENTITY_REQUEST_ATTACK:
                    {
                        i32 offset = 0;
                        i32 playerNumber = NetworkMessagePop<i32>( msg, offset );
                        EntityHandle target = NetworkMessagePop<EntityHandle>( msg, offset );
                        Action_Attack( playerNumber, target );
                    } break;
                }
            }
        }

        if( runSim == false ) {
            return;
        }

        dtAccumulator += dt;
        if( dtAccumulator < TURNS_DELTA ) {
            return;
        }
        dtAccumulator -= TURNS_DELTA;

        dt = TURNS_DELTA;

        //core->LogOutput( LogLevel::INFO, "SimMap::Update()" );

        EntList & entities = *core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        for( i32 entityIndexA = 0; entityIndexA < entities.GetCount(); entityIndexA++ ) {
            SimEntity * ent = entities[ entityIndexA ];
            if( ent->active == false ) {
                continue;
            }

            const f32 playerSpeed = 2500.0f / 100.0f;
            const f32 maxForce = 20.0f;
            const f32 invMass = 1.0f / 10.0f;

            // Boid arrival
            if( ent->type == EntityType::UNIT_TEST ) {
                if( ent->navigator.hasDest == true ) {
                    const glm::vec2 targetPos = ent->navigator.dest;
                    glm::vec2 desiredVel = ( targetPos - ent->pos );
                    f32 dist = glm::length( desiredVel );

                    if( dist < ent->navigator.slowRad ) {
                        if( dist < 5 ) {
                            desiredVel = glm::vec2( 0.0f );
                        }
                        else {
                            desiredVel = glm::normalize( desiredVel ) * playerSpeed * ( dist / ent->navigator.slowRad );
                        }
                    }
                    else {
                        desiredVel = glm::normalize( desiredVel ) * playerSpeed;
                    }

                #if 0
                    if( dist > ent->navigator.slowRad ) {
                        constexpr f32 maxSteerAngle = glm::radians( 15.0f );

                        glm::vec2 current = ent->vel;
                        if( glm::length2( current ) < 1.0f ) {
                            current = glm::vec2( glm::sin( ent->ori ), glm::cos( ent->ori ) );
                        }

                        f32 fullSteers = glm::acos( glm::clamp( glm::dot( glm::normalize( desiredVel ), glm::normalize( current ) ), -1.0f, 1.0f ) ) / maxSteerAngle;
                        f32 angl = glm::clamp( glm::sign( glm::dot( LeftPerp( desiredVel ), current ) ) * fullSteers, -1.0f, 1.0f );
                        ent->ori += angl * maxSteerAngle;

                        glm::vec2 face = glm::vec2( glm::sin( ent->ori ), glm::cos( ent->ori ) );
                        desiredVel = face * playerSpeed;
                    }
                #endif

                    if( glm::length2( ent->vel ) >= 1.0f ) {
                        glm::vec2 nvel = glm::normalize( ent->vel );
                        ent->ori = atan2f( nvel.x, nvel.y );
                    }


                    glm::vec2 steering = desiredVel - ent->vel;

                    steering = Truncate( steering, maxForce );
                    steering *= invMass;

                    ent->vel = Truncate( ent->vel + steering, playerSpeed );
                }

               /*
                f32 distToTarget = glm::length( desiredVel );
                if( distToTarget < ent->navigator.slowRad ) {
                    ent->acc = glm::vec2( 0.0f );
                    ent->vel = glm::normalize( toTarget ) * playerSpeed * ( distToTarget / ent->navigator.slowRad );
                }
                else {
                    ent->acc += glm::normalize( toTarget ) * playerSpeed;
                }*/

                Unit & unit = ent->unit;
                const i32 turretCount = unit.turrets.GetCount();
                for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
                    UnitTurret & turret = unit.turrets[ turretIndex ];
                    glm::vec2 worldPos = glm::rotate( turret.pos, ent->ori );
                    
                }

                for( i32 entityIndexB = 0; entityIndexB < entities.GetCount(); entityIndexB++ ) {
                    SimEntity * otherEnt = entities[ entityIndexB ];
                    f32 dist2 = glm::distance2( otherEnt->pos, ent->pos );
                    if( dist2 <= otherEnt->unit.averageFiringRange * otherEnt->unit.averageFiringRange ) {
                        
                    }
                }
            }

            //ent->acc.x -= ent->vel.x * ent->resistance;
            //ent->acc.y -= ent->vel.y * ent->resistance;
            ent->vel += ent->acc * dt;
            ent->pos += ent->vel * dt;

           
        }

        const i32 entityCount = entities.GetCount();
        if( core->NetworkIsConnected() == true ) {
            for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                SimEntity * ent = entities[ entityIndexA ];

                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                msg.type = NetworkMessageType::ENTITY_STATE_UPDATE;
                msg.isUDP = true;
                NetworkMessagePush( msg, ent->handle );
                u16 px = (u16)( ent->pos.x / MAX_MAP_SIZE * U16_MAX );
                u16 py = (u16)( ent->pos.y / MAX_MAP_SIZE * U16_MAX );
                //NetworkMessagePush( msg, ent->pos );
                NetworkMessagePush( msg, px );
                NetworkMessagePush( msg, py );
                NetworkMessagePush( msg, ent->ori );
                core->NetworkSend( msg );
            }
        }
    }

    SimEntity * SimMap::SpawnEntity( EntityType type, i32 playerNumber, i32 teamNumber, glm::vec2 pos, glm::vec2 vel ) {
        EntityHandle handle = {};
        SimEntity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->handle = handle;

            entity->active = true;
            entity->type = type;
            entity->resistance = 14.0f;
            entity->pos = pos;
            entity->vel = vel;
            entity->teamNumber = teamNumber;
            entity->playerNumber = playerNumber;

            switch( entity->type ) {
                case EntityType::UNIT_TEST:
                {
                    entity->selectionCollider.type = COLLIDER_TYPE_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 36 ) );

                    
                    entity->unit.averageFiringRange = 64.0f;

                } break;
            }

            communicator->OnEntitySpawn( entity );
        }

        return entity;
    }

    void SimMap::Action_Select( i32 playerNumber, EntHandleList & selection, ENTITY_SELECTION_CHANGE change ) {
        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        if( change == ENTITY_SELECTION_CHANGE_SET ) {
            const i32 entityCount = activeEntities.GetCount();
            for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = activeEntities[ entityIndex ];
                ent->selectedBy.RemoveValue( playerNumber );
            }
        }

        const i32 selectionCount = selection.GetCount();
        if( change == ENTITY_SELECTION_CHANGE_SET || change == ENTITY_SELECTION_CHANGE_ADD ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = selection[ entityHandleIndex ];
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->selectedBy.Add( playerNumber );
                }
            }
        }

        if( change == ENTITY_SELECTION_CHANGE_REMOVE ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = selection[ entityHandleIndex ];
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->selectedBy.RemoveValue( playerNumber );
                }
            }
        }
    }

    void SimMap::Action_Move( i32 playerNumber, glm::vec2 pos ) {
        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        const i32 entityCount = activeEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = activeEntities[ entityIndex ];
            if( ent->playerNumber == playerNumber && ent->selectedBy.Contains( playerNumber ) ) {
                ent->navigator.hasDest = true;
                ent->navigator.dest = pos;
                ent->navigator.slowRad = 100.0f;
            }
        }
    }

    void SimMap::Action_Attack( i32 playerNumber, EntityHandle target ) {
        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        SimEntity * targetEnt = entityPool.Get( target );
        if( targetEnt == nullptr ) {
            return;
        }

        const i32 entityCount = activeEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = activeEntities[ entityIndex ];
            if( ent->playerNumber == playerNumber && ent->selectedBy.Contains( playerNumber ) ) {
                ent->navigator.hasDest = true;
                glm::vec2 d = ( ent->pos - targetEnt->pos );
                f32 dist = glm::length( d );


                 if( dist > ent->unit.averageFiringRange - 10.0f ) {
                     glm::vec2 dir = d / dist;
                     ent->navigator.dest = targetEnt->pos + dir * ent->unit.averageFiringRange - 10.0f; // @HACK: The 10 is because firing range is from center ent so move in more forward for cannons at the back
                 } else {
                     ent->navigator.dest = ent->pos;
                 }

                ent->navigator.slowRad = 100.0f;
            }
        }
    }

    inline static Collider2D ColliderForSpace( const Collider2D & base, glm::vec2 p ) {
        Collider2D c = base;
        switch( base.type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                c.circle.pos += p;
            } break;
            case COLLIDER_TYPE_BOX:
            {
                c.box.Translate( p );
            } break;
            default:
            {
                INVALID_CODE_PATH;
            } break;
        }

        return c;
    }

    Collider2D SimEntity::GetWorldCollisionCollider() const {
        return ColliderForSpace( collisionCollider, pos );
    }

    Collider2D SimEntity::GetWorldSelectionCollider() const {
        return ColliderForSpace( selectionCollider, pos );
    }



    void SyncQueues::Start() {
        player1Turns.Clear();
        player2Turns.Clear();

        MapTurn turn = {};
        for( i32 i = 1; i <= slidingWindowWidth; i++ ) {
            turn.turnNumber = i;
            player1Turns.Enqueue( turn );
            player2Turns.Enqueue( turn );
        }
    }

    bool SyncQueues::CanTurn() {
        i32 player1Count = player1Turns.GetCount();
        i32 player2Count = player2Turns.GetCount();

        if( player1Count > 0 && player2Count > 0 ) {
            MapTurn * player1Turn = player1Turns.Peek();
            MapTurn * player2Turn = player2Turns.Peek();
            Assert( player1Turn->turnNumber == player2Turn->turnNumber );
            Assert( player1Turn->checkSum == player2Turn->checkSum );
            return true;
        }

        return false;
    }

    void SyncQueues::AddTurn( i32 playerNumber, const MapTurn & turn ) {
        if( playerNumber == 1 ) {
            player1Turns.Enqueue( turn );
        }
        else {
            player2Turns.Enqueue( turn );
        }
    }

    MapTurn * SyncQueues::GetNextTurn( i32 playerNumber ) {
        if( playerNumber == 1 ) {
            return player1Turns.Peek();
        }
        else {
            return player2Turns.Peek();
        }
    }

    void SyncQueues::FinishTurn() {
        player1Turns.Dequeue();
        player2Turns.Dequeue();
    }

}

